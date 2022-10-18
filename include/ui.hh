#pragma once

#include "types.hh"
#include <game_play.hh>

namespace c20::ui {

	class NoopUI : public core::UIHandler 
	{
		virtual void set_position(const commons::Position&);
		virtual void game_over();
	};

}
