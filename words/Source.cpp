#include "word_manager.h"
#include "console_helper.h"

#include <iostream>
#include <functional>

using namespace std;

int main()
{
	try
	{
		word_manager words{ "dict.xml" };
		auto end = false;

		while (!end)
		{
			run_command({
				{"Insert",				bind(&word_manager::insert, &words)				},
				{"Print",				bind(&word_manager::print, &words)				},
				{"Quiz",				bind(&word_manager::quiz, &words)				},
				{"Mark Learned Words",	bind(&word_manager::mark_learned_words, &words)	},
				{"Exit",				[&end]() {end = true; }							}
			}, false);
		}
	}
	catch (exception &e)
	{
		cout << e.what() << ". Exiting.\n";
	}
}
