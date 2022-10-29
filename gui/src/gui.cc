#include "widgets.hh"
#include <gui.hh>

namespace c20::gui {

	void GUI::game_over() {}

	void GUI::set_position(const Position& pos) 
	{
		window->set_position(pos);	
	}

	GUI::GUI(C2048Window *_window) : window(_window) {}

	void GUI::play_a_game()
	{
		game_player->play_a_game();
	}

}

