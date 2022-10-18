#pragma once

#include <types.hh>

namespace c20::core {


	using namespace commons;

	class MoveSelector
	{
		protected:
			Position pos;
		public:
			virtual void set_position(Position&);
			virtual UserMove make_move();
	};

	class GamePlayer;

	class UIHandler
	{
		private:
			GamePlayer *game_player;
		public:
			virtual void new_position(Position&);
			virtual void game_over();
	};

	
	enum State 
	{
		WAITING,
		RUNNING,
	};


	class GamePlayer 
	{
		private:
			Game game;
			MoveSelector *move_selector;
			UIHandler *ui;
			State current_state;
		public:
			/** Dumb stateless play one game method. */
			Game play_a_game();
	};


}
