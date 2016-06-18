#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>

using namespace std;

struct word
{
	string link;
	string s;
	string meaning;
	string example;
};

word read_word(ifstream& f, const string& link)
{
	auto line = string{};

	getline(f, line);

	if (line == "}")
		throw invalid_argument("");

	auto w = word{link};

	getline(f, w.s);
	getline(f, w.meaning);
	getline(f, w.example);

	return w;
}

void write_word(ofstream& f, const word& w)
{
	f << w.s << "\n";
	f << w.meaning << "\n";
	f << w.example << "\n";
}

vector<word> read_words_by_link(ifstream& f, const string& link)
{
	auto line = string{};
	auto res = vector<word>{};

	while (getline(f, line) && line == "{")
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
	cin >> res.s;

	if (res.s == "0")
		throw invalid_argument("");

	cout << "Meaning: ";
	cin >> res.meaning;

	cout << "Example: ";
	cin >> res.example;

	return res;
}

class word_manager
{
	vector<word> words;
	string dict_filename;

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
		cin >> link;

		while (1)
		{
			try
			{
				words.push_back(read_word_input(link));
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
		for_each(words.begin(), words.end(), [](auto& x) {
			cout << "Word: " << x.s << "\nMeaning: " << x.meaning << "\n\n";
		});
	}
};

int main()
{
	auto words = word_manager{ "dict.txt" };
	auto end = false;

	while (!end)
	{
		cout << "1. Insert\n";
		cout << "2. Print\n";
		cout << "3. Exit\n";
		cout << "Give your choice: ";

		auto x = int{};
		cin >> x;

		switch (x)
		{
		case 1:
			words.insert();
			break;
		case 2:
			words.print();
			break;
		case 3:
			end = true;
			break;
		default:
			break;
		}
	}
}
