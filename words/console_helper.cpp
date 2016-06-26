#include "console_helper.h"

#include <sstream>
#include <iostream>
#include <iterator>
#include <algorithm>

using namespace std;

template <typename T>
ostream& print_container(ostream& out, const vector<T>& v)
{
	if (!v.empty())
	{
		out << '[';
		copy(v.begin(), v.end(), ostream_iterator<T>(out, ", "));
		out << "\b\b]";
	}
	return out;
}

void get_user_input(const string& prompt, string& out)
{
	cout << prompt;
	getline(cin, out);
}

void get_user_input(const string& prompt)
{
	string temp;
	get_user_input(prompt, temp);
}

void get_user_input(const string& prompt, string& out, const vector<string>& possible_answers)
{
	while (1)
	{
		get_user_input(prompt, out);

		auto res = any_of(possible_answers.begin(), possible_answers.end(), [&out](auto& x) {
			return x == out;
		});

		if (res)
			break;
		else
		{
			cout << "Not valid answer. Possible answers: ";
			print_container(cout, possible_answers);
			cout << ".\n";
		}
	}
}

void get_user_input(const string& prompt, int& out)
{
	auto end = false;

	while (!end)
	{
		auto temp = string{};
		get_user_input(prompt, temp);
		try
		{
			out = stoi(temp);
			end = true;
		}
		catch (...)
		{
			cout << "Invalid option. Please choose a valid option.\n";
		}
	}
}

void get_user_input(const string& prompt, int& out, int min_allowed, int max_allowed)
{
	while (1)
	{
		get_user_input(prompt, out);

		if (out < min_allowed || out > max_allowed)
			cout << "Invalid option. Must be between " << min_allowed << " and " << max_allowed << ".\n";
		else
			break;
	}
}

string get_prompt_msg(const vector<command>& options)
{
	auto prompt_msg = stringstream{};
	auto count = 0;

	for_each(options.begin(), options.end(), [&count, &prompt_msg](auto& x) {
		prompt_msg << ++count << ". " << x.command_name << "\n";
	});

	prompt_msg << "Give your choice: ";

	return prompt_msg.str();
}

vector<command>::size_type get_option(const vector<command>& options)
{
	auto res = int{};

	get_user_input(get_prompt_msg(options), res, 1, static_cast<int>(options.size()));

	return res - 1;
}

void run_command(const vector<command>& _commands, bool has_return)
{
	auto commands = _commands;

	if (has_return)
		commands.insert(commands.begin(), { "Go Back", []() {} });

	commands[get_option(commands)].action();
}
