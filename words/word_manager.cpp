#include "word_manager.h"
#include "routines.h"
#include "console_helper.h"

#include <vector>

#include <intros_ptree.hpp>
#include <boost\property_tree\ptree.hpp>
#include <boost\property_tree\xml_parser.hpp>

using namespace std;
using namespace utils::intros_ptree;
using namespace boost::property_tree;

struct word_data
{
	string word;
	vector<string> links;
	string meaning;
	int got_right = 0;
	int total_quiz = 0;
	bool learned = false;

	word_data() {}
	explicit word_data(const string& link) :
		links({ link })
	{}
};

BEGIN_INTROS_TYPE(word_data)
	ADD_INTROS_ITEM(word)
	ADD_INTROS_ITEM(links)
	ADD_INTROS_ITEM(meaning)
	ADD_INTROS_ITEM(got_right)
	ADD_INTROS_ITEM(total_quiz)
	ADD_INTROS_ITEM(learned)
END_INTROS_TYPE(word_data)

struct word_container
{
	vector<word_data> words;

	vector<word_data>& operator()()
	{
		return words;
	}
};

BEGIN_INTROS_TYPE(word_container)
	ADD_INTROS_ITEM_USER_NAME(words, "word_data")
END_INTROS_TYPE(word_container)

word_data read_word_input(const string& link)
{
	word_data res{ link };

	get_user_input("Word (0 to end): ", res.word);

	if (res.word == "0")
		throw invalid_argument("");

	get_user_input("Meaning: ", res.meaning);

	return res;
}

void print_wrong_answers(const vector<word_data>& wrong_answers)
{
	if (!wrong_answers.empty())
	{
		cout << "You got the following " << wrong_answers.size() << " words wrong:\n";

		cout << '[';

		for_each(wrong_answers.begin(), wrong_answers.end(), [](auto& x) {
			cout << x.word << ", ";
		});

		cout << "\b\b]";
	}
}

struct filter
{
	vector<word_data>& cur_words;
	function<bool(const word_data& x)> f;

	filter(vector<word_data>& _cur_words, function<bool(const word_data& x)> _f) :
		cur_words(_cur_words),
		f(_f)
	{}

	void operator()()
	{
		cur_words.erase(remove_if(cur_words.begin(), cur_words.end(), f), cur_words.end());
	}
};

class end_quiz : public exception
{
public:
	end_quiz() {}
};

class word_manager::impl
{
private:
	word_container words;
	string dict_filename;

public:
	impl(const string& _dict_filename) :
		dict_filename(_dict_filename)
	{
		ptree tree;
		read_xml(dict_filename, tree);
		words = make_intros_object<word_container>(tree);
	}
	~impl()
	{
		xml_writer_settings<string> settings(' ', 4);
		auto os = ofstream(dict_filename);
		write_xml(os, make_ptree(words), settings);
	}

	void insert();
	void print();
	void quiz();
	void mark_learned_words();

private:
	vector<word_data>::iterator get_word(const string& name);
	vector<word_data> quiz_once(const vector<word_data>& words);
	void quiz_all_correct();

	// handle end_quiz exception for end quiz event
	void quiz_word(const word_data& x, function<void(vector<word_data>::iterator, bool)> callback_on_answer);
};

void word_manager::impl::insert()
{
	auto link = string{};

	get_user_input("Link: ", link);

	while (1)
	{
		try
		{
			auto new_word = read_word_input(link);

			auto old_word = get_word(new_word.word);

			if (old_word == words().end())
				words().push_back(new_word);
			else
			{
				old_word->links.push_back(link);
				old_word->meaning.append("; " + new_word.meaning);
				old_word->learned = false;
			}
		}
		catch (...)
		{
			// finished reading words...
			break;
		}
	}
}

void word_manager::impl::print()
{
	cout << "\n";

	auto cur_words = words;

	run_command({
		{ "All Words", []() {} },
		{ "Words Marked Learned", filter(cur_words(), [](const auto& x) {return !x.learned; }) },
		{ "Words Not Marked Learned", filter(cur_words(), [](const auto& x) {return x.learned; }) }
	});

	for_each(cur_words().begin(), cur_words().end(), [](auto& x) {
		cout << "Word: " << left << setw(15) << x.word << " Meaning: " << x.meaning << "\n";
	});

	cout << "\nTotal Words: " << cur_words().size() << "\n\n";
}

void word_manager::impl::quiz()
{
	cout << "\n";

	run_command({
		{ "One Word Once", bind(&word_manager::impl::quiz_once, this, cref(words())) },
		{ "Until You Get All Of Them Correct", bind(&word_manager::impl::quiz_all_correct, this) }
	});
}

void word_manager::impl::mark_learned_words()
{
	auto cur_words = words;

	cur_words().erase(remove_if(cur_words().begin(), cur_words().end(), [](auto& x) {
		return x.learned;
	}), cur_words().end());

	auto f = [](vector<word_data>::iterator word_it, bool res) {
		if (res)
		{
			string s;
			get_user_input("Mark word learned (y/n): ", s, { "y", "n" });
			if (s == "y")
				word_it->learned = true;
		}
	};

	try
	{
		for_each(cur_words().begin(), cur_words().end(), bind(&word_manager::impl::quiz_word, this, std::placeholders::_1, f));
	}
	catch (end_quiz&)
	{
		// quiz finished on user request... no action required
	}
}

vector<word_data>::iterator word_manager::impl::get_word(const string& name)
{
	return find_if(words().begin(), words().end(), [&name](auto& x) {
		return name == x.word;
	});
}

void word_manager::impl::quiz_word(const word_data& x, function<void(vector<word_data>::iterator, bool)> callback_on_answer)
{
	auto s = string{};
	get_user_input(stringer("\nWhat is the meaning of the word \"", x.word, "\" (0 to end quiz): "), s);

	if (s == "0")
		throw end_quiz();

	cout << "Compare your answer with the correct answer: " << x.meaning << "\n";

	get_user_input("Was your answer correct (y/n): ", s, { "y", "n" });

	callback_on_answer(get_word(x.word), s == "y");
}

vector<word_data> word_manager::impl::quiz_once(const vector<word_data>& words)
{
	auto wrong_answers = vector<word_data>{};

	auto f = [&wrong_answers](vector<word_data>::iterator word_it, bool res) {
		word_it->total_quiz++;
		if (res)
			word_it->got_right++;
		else
			wrong_answers.push_back(*word_it);
	};

	try
	{
		for_each(words.begin(), words.end(), bind(&word_manager::impl::quiz_word, this, std::placeholders::_1, f));
	}
	catch (end_quiz&)
	{
		// quiz finished on user request... no action required
	}

	cout << "\nFinished round.\n";

	cout << "You got " << (words.size() - wrong_answers.size()) << "/" << words.size() << " correct.\n";

	print_wrong_answers(wrong_answers);

	return wrong_answers;
}

void word_manager::impl::quiz_all_correct()
{
	auto round = 0;

	auto cur_words = words;

	run_command({
		{ "All Words", []() {} },
		{ "New Words", filter(cur_words(), [](auto& x) { return x.total_quiz != 0; }) },
		{ "Words Not Marked Learned", filter(cur_words(), [](auto& x) { return x.learned; }) }
	});

	shuffle_data(cur_words());

	while (!cur_words().empty())
	{
		get_user_input(stringer("\nPress enter to start round ", ++round, ": "));

		system("cls");

		cur_words() = quiz_once(cur_words());
	}

	cout << "You took " << round << " rounds to get all the answers correct.\n";
}

word_manager::word_manager(const string& _dict_filename) :
	pimpl(make_unique<impl>(_dict_filename))
{}

word_manager::~word_manager()
{}

void word_manager::insert()				{ pimpl->insert(); }
void word_manager::print()				{ pimpl->print(); }
void word_manager::quiz()				{ pimpl->quiz(); }
void word_manager::mark_learned_words()	{ pimpl->mark_learned_words(); }
