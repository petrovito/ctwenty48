#include "game_play.hh"
#include "selectors.hh"
#include "types.hh"
#include "ui.hh"
#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <ostream>


namespace c20::selectors {
	using namespace commons;

	TEST(Selector, RandomSelector)
	{
		RandomSelector selector;
		auto pos = Position::from_str("1234|5678|1234|0000");
		selector.set_position(pos);
		pos.calc_move(UP);
		pos.calc_move(DOWN);
		pos.calc_move(LEFT);
		pos.calc_move(RIGHT);
		ASSERT_EQ(DOWN, selector.make_move().direction);


		pos = Position::from_str("1234|5678|1234|5678");
		selector.set_position(pos);
		ASSERT_ANY_THROW(selector.make_move());

		pos = Position::from_str("0234|5678|1234|5678");
		selector.set_position(pos);
		bool did_up = false, did_left = false;
		for (int i = 0; i < 100; i++)
		{
			auto move = selector.make_move();
			ASSERT_TRUE(move.direction == LEFT || move.direction == UP);
			if (move.direction == LEFT) did_left = true;
			else did_up = true;
		}
		ASSERT_TRUE(did_left && did_up);
	}


	TEST(Selector, PlayRandomGame)
	{
		auto ui = std::shared_ptr<ui::UIHandler>(new ui::NoopUI());
		auto random_selector = 
			std::shared_ptr<core::MoveSelector>(new RandomSelector());
		core::GamePlayer game_player(ui, random_selector);
		auto game = game_player.play_a_game();
		ASSERT_LT(32, game->history().size());
		ASSERT_TRUE(game->current_position()->is_over());
	}

}
