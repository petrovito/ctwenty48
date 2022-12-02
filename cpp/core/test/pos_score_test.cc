#include "types.hh"
#include <pos_score.hh>
#include <gtest/gtest.h>

namespace c20::search {

	TEST(PosScoreTest, MainPath)
	{
		StaticPositionEval pos_eval;
		{
			auto main_path = 
				pos_eval.find_main_path(Position::from_str("1111|2222|3333|4561"));
			ASSERT_EQ(0, main_path.len);
			ASSERT_EQ(0, main_path.breaker_num);
			ASSERT_EQ(1, main_path.optimal_num);
		}
		{
			auto main_path = 
				pos_eval.find_main_path(Position::from_str("1111|2222|3333|4527"));
			ASSERT_EQ(1, main_path.len);
			ASSERT_NE(0, main_path.optimal_num);
		}
		{
			auto main_path = 
				pos_eval.find_main_path(Position::from_str("1111|2222|3333|2567"));
			ASSERT_EQ(3, main_path.len);
			ASSERT_EQ(2, main_path.breaker_num);
			ASSERT_EQ(3, main_path.optimal_num);
		}
		{
			auto main_path = 
				pos_eval.find_main_path(Position::from_str("1111|2222|2333|4567"));
			ASSERT_EQ(4, main_path.len);
			ASSERT_EQ(2, main_path.breaker_num);
			ASSERT_EQ(3, main_path.optimal_num);
		}
		{
			auto main_path = 
				pos_eval.find_main_path(Position::from_str("1111|2222|3333|4567"));
			ASSERT_EQ(5, main_path.len);
			ASSERT_EQ(0, main_path.breaker_num);
			ASSERT_EQ(1, main_path.optimal_num);
		}	
		{
			auto main_path = 
				pos_eval.find_main_path(Position::from_str("1111|2222|0003|7654"));
			ASSERT_EQ(5, main_path.len);
			ASSERT_EQ(0, main_path.breaker_num);
			ASSERT_EQ(1, main_path.optimal_num);
		}		
		{
			auto main_path = 
				pos_eval.find_main_path(Position::from_str("7111|6222|5222|4300"));
			ASSERT_EQ(5, main_path.len);
			ASSERT_EQ(0, main_path.breaker_num);
			ASSERT_EQ(1, main_path.optimal_num);
		}		{
			auto main_path = 
				pos_eval.find_main_path(Position::from_str("1117|6522|2222|0300"));
			ASSERT_EQ(1, main_path.len);
		}
	}

}
