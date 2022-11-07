
#include "types.hh"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fcntl.h>
#include <fstream>
#include <game_logger.hh>
#include <gtest/gtest.h>
#include <sys/stat.h>

#include <ext/stdio_filebuf.h>  // __gnu_cxx::stdio_filebuf
#include <unistd.h>

namespace c20::misc {
using namespace std;
	TEST(GameLogger, SimpleLog)
	{
		Game game(Position::from_str("1234|0000|4321|7895"));
		{
			int pipe_pair[2];
			if (pipe(pipe_pair) < 0) FAIL();
			__gnu_cxx::stdio_filebuf<char> fb(pipe_pair[0], std::ios_base::in);
			std::istream istream(&fb);
			char buffer[120];


			{
				GameLogger logger(pipe_pair[1]);
				logger.log_game(game);
			}
			istream.readsome(buffer, 120);
			ASSERT_STREQ("1,2,3,4,0,0,0,0,4,3,2,1,7,8,9,5,1,0,1\n", buffer);
		}


		game.do_move(UserMove{.direction=DOWN});
		{
			int pipe_pair[2];
			if (pipe(pipe_pair) < 0) FAIL();
			__gnu_cxx::stdio_filebuf<char> fb(pipe_pair[0], std::ios_base::in);
			std::istream istream(&fb);
			char buffer[120];


			{
				GameLogger logger(pipe_pair[1]);
				logger.log_game(game);
			}
			istream.readsome(buffer, 120);
			//assert only one differs:
			int diffs = 0;
			std::string expected = "1,2,3,4,0,0,0,0,4,3,2,1,7,8,9,5,1,1,2\n"
				"0,0,0,0,1,2,3,4,4,3,2,1,7,8,9,5,2,0,2\n";
			for (int i = 0; i < expected.size(); i++)
			{
				if (expected[i] != buffer[i]) diffs++;
			}
			ASSERT_EQ(1, diffs);
		}
	}
}

