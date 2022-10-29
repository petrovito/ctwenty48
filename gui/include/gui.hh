#pragma once

#include "widgets.hh"
#include <memory>
#include <game_play.hh>
#include <ui.hh>

namespace c20::gui {
	using namespace commons;

	class GUI : public ui::UIHandler
	{
		private:
			C2048Window* window;
		public:
			GUI(C2048Window*);

			virtual void set_position(const Position&);
			virtual void game_over();

			void play_a_game();
	};

}
