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
		//todo
	}


	const Position* Game::current_position() 
	{
		return this->current_pos;
	}

	GeneralDirection general(MoveDirection direction) 
	{
		return GeneralDirection(direction >> 1);
	}


}


