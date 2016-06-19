#include <map>
#include <random>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <functional>

using namespace std;
using namespace std::placeholders;

struct command
{
	string command_name;
	function<void(void)> action;
};

auto get_option(const vector<command>& options) -> vector<command>::size_type
{
	const auto wrong_answer = options.size() + 1;
	auto res = wrong_answer;

	while (res == wrong_answer)
	{
		auto count = 0;

		for_each(options.begin(), options.end(), [&count](auto& x) {
			cout << ++count << ". " << x.command_name << "\n";
		});

		cout << "Give your choice: ";

		auto temp = string{};
		getline(cin, temp);

		try
		{
			res = stoi(temp);
			if (res <= 0 || res > options.size())
				throw invalid_argument("");
		}
		catch (...)
		{
			res = wrong_answer;
			cout << "Invalid option. Please choose a valid option.\n";
		}
	}

	return res - 1;
}

void run_command(const vector<command>& commands)
{
	commands[get_option(commands)].action();
}

template<typename T>
void shuffle_data(T& cont)
{
	random_device rd;
	mt19937 g(rd());
	shuffle(cont.begin(), cont.end(), g);
}

struct word
{
	string link;
	string s;
	string meaning;
	int got_right = 0;
	int total_quiz = 0;

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

	ss >> w.got_right >> ch >> w.total_quiz;

	return w;
}

void write_word(ofstream& f, const word& w)
{
	f << w.s << "\n";
	f << w.meaning << "\n";
	f << w.got_right << "/" << w.total_quiz << "\n";
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

	cout << "Word (0 to end): ";
	getline(cin, res.s);

	if (res.s == "0")
		throw invalid_argument("");

	cout << "Meaning: ";
	getline(cin, res.meaning);

	return res;
}

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

	void quiz_word(const word& x, int& res, vector<word>& wrong_answers)
	{
		cout << "\nWhat is the meaning of the word \"" << x.s << "\": ";
		auto s = string{};
		getline(cin, s);
		cout << "Compare your answer with the correct answer: " << x.meaning << "\n";
		cout << "Was your answer correct (y/n): ";
		getline(cin, s);

		auto it = get_word(x.s);
		it->total_quiz++;

		if (s == "y")
		{
			res += 1;
			it->got_right++;
		}
		else
		{
			wrong_answers.push_back(x);
		}
	}
	
	// each word asked once
	vector<word> quiz_once(const vector<word>& words)
	{
		auto res = 0;
		auto wrong_answers = vector<word>{};

		for_each(words.begin(), words.end(), bind(&word_manager::quiz_word, this, _1, ref(res), ref(wrong_answers)));

		cout << "You got " << res << "/" << words.size() << " correct.\n";

		return wrong_answers;
	}

	// until you give correct answer for all words
	void quiz_all_correct()
	{
		auto round = 0;

		auto cur_words = words;

		shuffle_data(cur_words);

		while (!cur_words.empty())
		{
			system("cls");

			cout << "Round " << ++round << "\n";

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
		cout << "Link: ";
		getline(cin, link);

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

		for_each(words.begin(), words.end(), [](auto& x) {
			cout << "Word: " << left << setw(15) << x.s << " Meaning: " << x.meaning << "\n";
		});

		cout << "\n";
	}

	void quiz()
	{
		cout << "\n";

		run_command({
			{"One word once", bind(&word_manager::quiz_once, this, cref(words))},
			{"Until you get all of them correct", bind(&word_manager::quiz_all_correct, this)}
		});
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
			{"Exit", [&end]() {end = true; }}
		});
	}
}
