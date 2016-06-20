#pragma once


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

template <typename T>
std::ostream& operator<< (std::ostream& out, const std::vector<T>& v)
{
	if (!v.empty())
	{
		out << '[';
		std::copy(v.begin(), v.end(), std::ostream_iterator<T>(out, ", "));
		out << "\b\b]";
	}
	return out;
}
