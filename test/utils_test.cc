#include <gtest/gtest.h>
#include <numeric>
#include <types.hh>
#include <vector>

namespace c20::commons {

	TEST(PositionTest, FromString) 
	{
		Position pos = Position::from_str("0000|0000|1200|0000");
		ASSERT_EQ(0, pos(0,0));
		ASSERT_EQ(1, pos(2,0));
		ASSERT_EQ(2, pos(2,1));
		ASSERT_EQ(1, pos[8]);
		ASSERT_EQ(2, pos[9]);
		ASSERT_EQ(0, pos[10]);
	}

	TEST(PositionTest, PosMove) 
	{
		Position pos = Position::from_str("0000|0000|1200|0000");
		auto move_result = pos.calc_move(LEFT);
		ASSERT_FALSE(move_result.has_changed);

		pos = Position::from_str("0000|0000|1202|0000");
		move_result = pos.calc_move(LEFT);
		ASSERT_TRUE(move_result.has_changed);
		auto [new_pos, zeros] = move_result.calc_pos_zeros_pair();
		ASSERT_EQ(1, new_pos[8]);
		ASSERT_EQ(3, new_pos[9]);

		pos = Position::from_str("1111|0000|0000|0000");
		move_result = pos.calc_move(LEFT);
		ASSERT_TRUE(move_result.has_changed);
		std::tie(new_pos, zeros) = move_result.calc_pos_zeros_pair();
		ASSERT_EQ(2, new_pos[0]);
		ASSERT_EQ(2, new_pos[1]);
		ASSERT_EQ(0, new_pos[2]);
		ASSERT_EQ(0, new_pos[3]);
		
		pos = Position::from_str("1234|0000|0000|0000");
		move_result = pos.calc_move(LEFT);
		ASSERT_FALSE(move_result.has_changed);
		move_result = pos.calc_move(UP);
		ASSERT_FALSE(move_result.has_changed);
		move_result = pos.calc_move(DOWN);
		ASSERT_TRUE(move_result.has_changed);
	}
	
	TEST(PositionTest, PopNumber)
	{
		auto pos = Position::from_str("1234|0000|0000|0000");
		NumberPopper popper;
		for (int i = 0; i < 100; i++) 
		{
			auto popped = popper.pop(12);		
			ASSERT_TRUE(12 > popped.idx);
			ASSERT_TRUE(1 == popped.value || 2 == popped.value);
		}
		ZeroIndices zeros{4,5,6,7,8,9,10,11,12,13,14,15};
		popper.place_one(pos, zeros);
		
		int changed_entries = 0;
		for (int i = 4; i < 16; i++)
		{
			if (pos[i]) changed_entries++;
		}
		ASSERT_EQ(1, changed_entries);

		//test dist from
		pos = Position::from_str("1234|1234|1234|0000");
		zeros = {12,13,14,15};
		auto dist = popper.dist_from(pos, zeros);
		ASSERT_EQ(8, dist.size());
		Probability prob_pop_two = .75/4, prob_pop_four = .25/4;
		for (auto [prob, new_pos]: dist) 
		{
			ASSERT_EQ(3, new_pos.num_zeros());
			Number popped_val = 0;
			for (int i = 12; i < 16; i++) popped_val += new_pos[i];
			ASSERT_TRUE(popped_val == 1 || popped_val == 2);
			if (popped_val == 1) ASSERT_NEAR(prob_pop_two, prob, 1e-5);
			else ASSERT_NEAR(prob_pop_four, prob, 1e-5);
		}
	}	

	TEST(PositionTest, IsOver)
	{
		auto pos = Position::from_str("1234|0000|0000|0000");
		ASSERT_FALSE(pos.is_over());	
		//no zeros no merges
		pos = Position::from_str("1234|5678|1234|5678");
		ASSERT_TRUE(pos.is_over());	
		//no zeros but merges!
		pos = Position::from_str("1234|1234|1234|5678");
		ASSERT_FALSE(pos.is_over());	
	}

	TEST(PositionTest, StartGame)
	{
		Game game = Game::start_game();
		ASSERT_EQ(NUM_SQUARES -2, game.current_position() -> num_zeros());
	}
	
}

