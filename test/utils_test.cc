#include <gtest/gtest.h>
#include <types.hh>

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
		auto pos_zeros = move_result.calc_pos_zeros_pair();
		ASSERT_EQ(1, pos_zeros.pos[8]);
		ASSERT_EQ(3, pos_zeros.pos[9]);

		pos = Position::from_str("1111|0000|0000|0000");
		move_result = pos.calc_move(LEFT);
		ASSERT_TRUE(move_result.has_changed);
		pos_zeros = move_result.calc_pos_zeros_pair();
		ASSERT_EQ(2, pos_zeros.pos[0]);
		ASSERT_EQ(2, pos_zeros.pos[1]);
		ASSERT_EQ(0, pos_zeros.pos[2]);
		ASSERT_EQ(0, pos_zeros.pos[3]);
		
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
		PopPlacer placer{.pos=&pos, .zeros=&zeros, .popper=&popper};
		placer.place_one();
		
		int changed_entries = 0;
		for (int i = 4; i < 16; i++)
		{
			if (pos[i]) changed_entries++;
		}
		ASSERT_EQ(1, changed_entries);
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
}

