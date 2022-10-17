
#pragma once

#include <fstream>
#include <string>
#include <types.hh>
#include <ext/stdio_filebuf.h>  // __gnu_cxx::stdio_filebuf

namespace c20::misc {

	using namespace commons;


	class GameLogger
	{
		private:
			std::FILE *fd;
			__gnu_cxx::stdio_filebuf<char> buffer;
			std::ostream ostream;
		public:
			GameLogger(std::string&& path, bool append=true);
			GameLogger(int _fd, bool append=true);
			~GameLogger();
			void log_game(Game&);
	};

}
