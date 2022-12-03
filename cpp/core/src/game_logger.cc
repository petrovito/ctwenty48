
#include "types.hh"
#include <cstdio>
#include <game_logger.hh>
#include <ios>
#include <csv.hpp>
#include <vector>

#include <ext/stdio_filebuf.h>  // __gnu_cxx::stdio_filebuf

namespace c20::misc {



	GameLogger::GameLogger(const std::string& path, bool append) :
		fd(fopen(path.c_str(), append ? "a" : "w")),
		buffer(fd->_fileno, append ? std::ios_base::app : std::ios_base::out),
		ostream(&buffer)
	{ }


	GameLogger::GameLogger(int _fd, bool append) :
		fd(nullptr),
		buffer(_fd, append ? std::ios_base::app : std::ios_base::out),
		ostream(&buffer)
	{ }

	GameLogger::~GameLogger()
	{
		if (fd) delete fd;
	}

	void GameLogger::log_game(Game& game)
	{
		auto writer = csv::make_csv_writer(ostream);
		auto iterator = game.history();
		auto pos = iterator.begin();
		for (unsigned i = 0; i < iterator.size(); i++, pos++)
		{
			std::vector<int> row;
			//append position table
			for (auto num: pos->squares())
			{
				row.push_back(num);
			}
			//append move number
			row.push_back(i +1);
			//append moves left
			row.push_back(iterator.size() - i -1);
			//append game length
			row.push_back(iterator.size());

			writer << row;
		}
	}

}


