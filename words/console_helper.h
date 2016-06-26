#pragma once

#include <vector>
#include <functional>

void get_user_input(const std::string& prompt, std::string& out);
void get_user_input(const std::string& prompt);
void get_user_input(const std::string& prompt, std::string& out, const std::vector<std::string>& possible_answers);
void get_user_input(const std::string& prompt, int& out);
void get_user_input(const std::string& prompt, int& out, int min_allowed, int max_allowed);

struct command
{
	std::string command_name;
	std::function<void(void)> action;
};

void run_command(const std::vector<command>& _commands, bool has_return = true);
