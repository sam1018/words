#pragma once

#include <memory>
#include <string>

class word_manager
{
public:
	explicit word_manager(const std::string& _dict_filename);
	~word_manager();

	void insert();
	void print();
	void quiz();
	void mark_learned_words();

private:
	class impl;
	std::unique_ptr<impl> pimpl;
};
