#pragma once

#include <random>
#include <string>
#include <vector>
#include <iostream>


template< typename ... Args >
std::string stringer(Args const& ... args)
{
	std::ostringstream stream;
	using List = int[];
	(void)List {
		0, ((void)(stream << args), 0) ...
	};

	return stream.str();
}

template<typename T>
void shuffle_data(T& cont)
{
	random_device rd;
	mt19937 g(rd());
	shuffle(cont.begin(), cont.end(), g);
}
