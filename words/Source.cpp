#include "routines.h"
#include "console_helper.h"

#include <map>
#include <random>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <functional>

using namespace std;
using namespace std::placeholders;

struct word
{
	string link;
	string s;
	string meaning;
	int got_right = 0;
	int total_quiz = 0;
	bool learned = false;

	explicit word(const string& _link) :
		link(_link)
	{}
};

word read_word(ifstream& f, const string& link)
{
	auto w = word{ link };

	getline(f, w.s);

	if (w.s == "}")
		throw invalid_argument("");

	getline(f, w.meaning);

	auto temp = string{};

	getline(f, temp);

	stringstream ss(temp);

	auto ch = char{};

	ss >> w.got_right >> ch >> w.total_quiz >> ch >> w.learned;

	return w;
}

void write_word(ofstream& f, const word& w)
{
	f << w.s << "\n";
	f << w.meaning << "\n";
	f << w.got_right << "/" << w.total_quiz << "/" << w.learned << "\n";
}

vector<word> read_words_by_link(ifstream& f, const string& link)
{
	auto line = string{};
	auto res = vector<word>{};

	// read "{" and discard
	getline(f, line);

	while (1)
	{
		try
		{
			res.push_back(read_word(f, link));
		}
		catch (...)
		{
			// finished reading...
			break;
		}
	}

	return res;
}

void write_words_by_link(ofstream& f, const vector<word>& words)
{
	if (!words.empty())
	{
		f << words[0].link << "\n";
		f << "{\n";

		for_each(words.begin(), words.end(), [&f](auto& x) {
			write_word(f, x);
		});

		f << "}\n";
	}
}

word read_word_input(const string& link)
{
	word res{ link };

	get_user_input("Word (0 to end): ", res.s);

	if (res.s == "0")
		throw invalid_argument("");

	get_user_input("Meaning: ", res.meaning);

	return res;
}

void print_wrong_answers(const vector<word>& wrong_answers)
{
	if (!wrong_answers.empty())
	{
		cout << "You got the following " << wrong_answers.size() << " words wrong:\n";

		cout << '[';

		for_each(wrong_answers.begin(), wrong_answers.end(), [](auto& x) {
			cout << x.s << ", ";
		});

		cout << "\b\b]";
	}
}

struct filter
{
	vector<word>& cur_words;
	function<bool(const word& x)> f;

	filter(vector<word>& _cur_words, function<bool(const word& x)> _f) :
		cur_words(_cur_words),
		f(_f)
	{}

	void operator()()
	{
		cur_words.erase(remove_if(cur_words.begin(), cur_words.end(), f), cur_words.end());
	}
};

class word_manager
{
	// words will maintain the following properties:
	// 1. no duplicate words
	vector<word> words;
	string dict_filename;

private:
	vector<word>::iterator get_word(const string& name)
	{
		return find_if(words.begin(), words.end(), [&name](auto& x) {
			return name == x.s;
		});
	}

	void quiz_word(const word& x, function<void(vector<word>::iterator, bool)> callback_on_answer)
	{
		auto s = string{};
		get_user_input(stringer("\nWhat is the meaning of the word \"", x.s, "\": "), s);

		cout << "Compare your answer with the correct answer: " << x.meaning << "\n";

		get_user_input("Was your answer correct (y/n): ", s, { "y", "n" });

		callback_on_answer(get_word(x.s), s == "y");
	}
	
	// each word asked once
	vector<word> quiz_once(const vector<word>& words)
	{
		auto wrong_answers = vector<word>{};

		auto f = [&wrong_answers](vector<word>::iterator word_it, bool res) {
			word_it->total_quiz++;
			if (res)
				word_it->got_right++;
			else
				wrong_answers.push_back(*word_it);
		};

		for_each(words.begin(), words.end(), bind(&word_manager::quiz_word, this, _1, f));

		cout << "\nFinished round.\n";

		cout << "You got " << (words.size() - wrong_answers.size()) << "/" << words.size() << " correct.\n";

		print_wrong_answers(wrong_answers);

		return wrong_answers;
	}

	// until you give correct answer for all words
	void quiz_all_correct()
	{
		auto round = 0;

		auto cur_words = words;

		run_command({
			{ "All Words", []() {} },
			{ "New Words", filter(cur_words, [](auto& x) { return x.total_quiz != 0; }) },
			{ "Words Not Marked Learned", filter(cur_words, [](auto& x) { return x.learned; }) }
		});

		shuffle_data(cur_words);

		while (!cur_words.empty())
		{
			get_user_input(stringer("\nPress enter to start round ", ++round, ": "));

			system("cls");

			cur_words = quiz_once(cur_words);
		}

		cout << "You took " << round << " rounds to get all the answers correct.\n";
	}

public:
	word_manager(const string& _dict_filename) :
		dict_filename(_dict_filename)
	{
		auto f = ifstream(dict_filename);
		auto line = string{};

		while (getline(f, line))
		{
			auto link = line;
			auto new_words = read_words_by_link(f, link);
			words.insert(words.end(), new_words.begin(), new_words.end());
		}
	}

	~word_manager()
	{
		auto f = ofstream(dict_filename);

		auto map_by_links = map<string, vector<word>>{};

		for_each(words.begin(), words.end(), [&map_by_links](auto& x) {
			auto& item = map_by_links[x.link];
			item.push_back(x);
		});

		for_each(map_by_links.begin(), map_by_links.end(), [&f](auto& x) {
			write_words_by_link(f, x.second);
		});
	}

	void insert()
	{
		auto link = string{};

		get_user_input("Link: ", link);

		while (1)
		{
			try
			{
				auto new_word = read_word_input(link);

				if (get_word(new_word.s) == words.end())
					words.push_back(new_word);
				else
					cout << "Already in the dictionary.\n";
			}
			catch (...)
			{
				// finished reading words...
				break;
			}
		}
	}

	void print()
	{
		cout << "\n";

		auto cur_words = words;

		run_command({
			{"All Words", []() {}},
			{"Words Marked Learned", filter(cur_words, [](const word& x) {return !x.learned; })},
			{"Words Not Marked Learned", filter(cur_words, [](const word& x) {return x.learned; }) }
		});

		for_each(cur_words.begin(), cur_words.end(), [](auto& x) {
			cout << "Word: " << left << setw(15) << x.s << " Meaning: " << x.meaning << "\n";
		});

		cout << "\nTotal Words: " << cur_words.size() << "\n\n";
	}

	void quiz()
	{
		cout << "\n";

		run_command({
			{"One Word Once", bind(&word_manager::quiz_once, this, cref(words))},
			{"Until You Get All Of Them Correct", bind(&word_manager::quiz_all_correct, this)}
		});
	}

	void mark_learned_words()
	{
		auto cur_words = words;

		cur_words.erase(remove_if(cur_words.begin(), cur_words.end(), [](auto& x) {
			return x.learned;
		}), cur_words.end());

		auto f = [](vector<word>::iterator word_it, bool res) {
			if (res)
			{
				string s;
				get_user_input("Mark word learned (y/n): ", s, { "y", "n" });
				if (s == "y")
					word_it->learned = true;
			}
		};

		for_each(cur_words.begin(), cur_words.end(), bind(&word_manager::quiz_word, this, _1, f));
	}
};

int main()
{
	auto words = word_manager{ "dict.txt" };
	auto end = false;

	while (!end)
	{
		run_command({
			{"Insert", bind(&word_manager::insert, &words)},
			{"Print", bind(&word_manager::print, &words)},
			{"Quiz", bind(&word_manager::quiz, &words)},
			{"Mark Learned Words", bind(&word_manager::mark_learned_words, &words)},
			{"Exit", [&end]() {end = true; }}
		});
	}
}
