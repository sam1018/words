#pragma once

#include <sstream>
#include <algorithm>
#include <functional>

void get_user_input(const std::string& prompt, std::string& out)
{
	std::cout << prompt;
	std::getline(std::cin, out);
}

void get_user_input(const std::string& prompt)
{
	std::string temp;
	get_user_input(prompt, temp);
}

void get_user_input(const std::string& prompt, std::string& out, const std::vector<std::string>& possible_answers)
{
	while (1)
	{
		get_user_input(prompt, out);

		auto res = std::any_of(possible_answers.begin(), possible_answers.end(), [&out](auto& x) {
			return x == out;
		});

		if (res)
			break;
		else
			std::cout << "Not valid answer. Possible answers: " << possible_answers << ".\n";
	}
}

void get_user_input(const std::string& prompt, int& out)
{
	auto end = false;

	while (!end)
	{
		auto temp = std::string{};
		get_user_input(prompt, temp);
		try
		{
			out = std::stoi(temp);
			end = true;
		}
		catch (...)
		{
			std::cout << "Invalid option. Please choose a valid option.\n";
		}
	}
}

void get_user_input(const std::string& prompt, int& out, int min_allowed, int max_allowed)
{
	while (1)
	{
		get_user_input(prompt, out);

		if (out < min_allowed || out > max_allowed)
			std::cout << "Invalid option. Must be between " << min_allowed << " and " << max_allowed << ".\n";
		else
			break;
	}
}

struct command
{
	std::string command_name;
	std::function<void(void)> action;
};

std::string get_prompt_msg(const std::vector<command>& options)
{
	auto prompt_msg = std::stringstream{};
	auto count = 0;

	std::for_each(options.begin(), options.end(), [&count, &prompt_msg](auto& x) {
		prompt_msg << ++count << ". " << x.command_name << "\n";
	});

	prompt_msg << "Give your choice: ";

	return prompt_msg.str();
}

std::vector<command>::size_type get_option(const std::vector<command>& options)
{
	auto res = int{};

	get_user_input(get_prompt_msg(options), res, 1, static_cast<int>(options.size()));

	return res - 1;
}

void run_command(const std::vector<command>& commands)
{
	commands[get_option(commands)].action();
}
