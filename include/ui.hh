#pragma once

#include "types.hh"

namespace c20::core { class GamePlayer; }

namespace c20::ui {
	using namespace commons;


	class UIHandler
	{
		protected:
			core::GamePlayer *game_player;
		public:
			virtual void set_position(const Position&) = 0;
			virtual void game_over() = 0;
	};


	class NoopUI : public UIHandler 
	{
		virtual void set_position(const commons::Position&);
		virtual void game_over();
	};


}
