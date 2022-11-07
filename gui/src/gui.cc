#include "game_play.hh"
#include "types.hh"
#include <gui.hh>
#include <boost/asio.hpp>

#include <future>
#include <memory>
#include <thread>

namespace c20::gui {

//GuiMessageChannel

	void GuiMessageChannel::message_to(AppComponent comp, GuiMessage msg)
	{
		message_q_pair[comp].push(msg);
	}

	GuiMessage GuiMessageChannel::message_from(AppComponent comp)
	{
		GuiMessage msg;
		message_q_pair[1-comp].pop(msg);
		return msg;
	}

//BackendConnector

	void BackendConnector::game_over() {}

	void BackendConnector::set_position(const Position& pos) 
	{
		channel->message_to(FRONTEND, {.action=SET_POSITION, .pos=pos});
	}

	BackendConnector::BackendConnector(core::GamePlayer* _game_player,
			GuiMessageChannel* _channel) :
		UIHandler(_game_player), channel(_channel)
	{ 
		msg_receiver_thread = std::make_unique<std::thread>(
				&BackendConnector::receive_messages, this);
	}


	void BackendConnector::receive_messages()
	{
		while (!shutting_down) {
			GuiMessage message = channel->message_from(FRONTEND);
			switch (message.action) {
			case START_GAME:
				game_player->play_a_game(); //TODO async
				break;
			}
		}
	}

//FrontendConnector


	FrontendConnector::FrontendConnector(GuiMessageChannel* _channel, 
			StateInfoHandler* _handler) :
		channel(_channel), handler(_handler)
	{ 
		msg_receiver_thread = std::make_unique<std::thread>(
				&FrontendConnector::receive_messages, this);
	}

	void FrontendConnector::receive_messages()
	{
		while (!shutting_down) {
			GuiMessage message = channel->message_from(FRONTEND);
			switch (message.action) {
			case SET_POSITION:
				handler->set_position(message.pos);
				break;
			}
		}
	}

	void FrontendConnector::play_a_game()
	{
		channel->message_to(BACKEND, {START_GAME});
	}


//StateInfoHandler

	void StateInfoHandler::set_position(const Position& pos)
	{
		state_info.current_pos = pos;
		window->set_position(pos);
	}

}

