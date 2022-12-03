#include "types.hh"
#include <ui.hh>

namespace c20::ui {

	UIHandler::UIHandler(core::GamePlayer* _game_player) :
		game_player(_game_player) {}

	void UIHandler::set_game_player(core::GamePlayer* _game_player)
	{
		game_player = _game_player;
	}

	void NoopUI::set_position(const commons::Position&) {}
	void NoopUI::game_over() {}
	void NoopUI::analysis_msg(const Analysis&, message_key) {}



}
