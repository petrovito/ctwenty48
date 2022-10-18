#pragma once

#include <game_play.hh>

namespace c20::ui {

	class NoopUI : public core::UIHandler 
	{
		virtual void new_position();
		virtual void game_over();
	};

}
