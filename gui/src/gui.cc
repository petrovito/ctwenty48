#include "game_play.hh"
#include <gui.hh>
#include <boost/asio.hpp>

#include <future>

namespace c20::gui {

	void GUI::game_over() {}

	void GUI::set_position(const Position& pos) 
	{
		window->set_position(pos);	
	}

	GUI::GUI(core::GamePlayer* _game_player, C2048Window *_window) :
		UIHandler(_game_player), window(_window), pool(4) 
	{
		boost::asio::post(pool, [this] {
				receive_messages();
				});
	}

	GUI::GUI(C2048Window *_window) :
		GUI(nullptr, _window) {}

	void GUI::play_a_game()
	{
		messages.push({START_GAME});
	}


	void GUI::receive_messages()
	{
		while (1) {
			GuiMessage message;
			messages.pop(message);
			switch (message.action) {
			case START_GAME:
				boost::asio::post(pool, [this] {
						game_player->play_a_game();
						});
				break;
			}
		}
	}


}

