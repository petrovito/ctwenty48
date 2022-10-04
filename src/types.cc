#include <types.hh>

namespace c20::commons {

	Game::Game() :
		current_pos_idx(0)
	{
		//todo
	}


	Game::Game(Position pos)
	{
		//todo
	}


	MoveResult Game::do_move(UserMove user_move) 
	{
		auto dir = user_move.direction;
		auto effect = current_pos->merges_along(dir);
		Position new_pos(effect, dir);
	}


	const Position* Game::current_position() 
	{
		return this->current_pos;
	}

	GeneralDirection general(MoveDirection direction) 
	{
		return GeneralDirection(direction >> 1);
	}

	int index_table[][TABLE_SIZE][TABLE_SIZE] = {
		//UP
		{
			{1, 2, 3, 4},
		},
	};





}


