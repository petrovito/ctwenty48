#include "game_play.hh"
#include "types.hh"
#include <gui.hh>
#include <boost/asio.hpp>

#include <future>
#include <memory>
#include <thread>

#include <spdlog/spdlog.h>

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

	void BackendConnector::init()
	{ 
		msg_receiver_thread = std::make_unique<std::thread>(
				&BackendConnector::receive_messages, this);
	}


	void BackendConnector::receive_messages()
	{
		while (!shutting_down) {
			GuiMessage message = channel->message_from(FRONTEND);
			SPDLOG_DEBUG("Message from frontend received. Action {}", message.action);
			switch (message.action) {
			case START_GAME:
				SPDLOG_INFO("Frontend request: Start a game.");
				game_player->play_a_game(); //TODO async
				break;
			}
		}
	}

//FrontendConnector


	void FrontendConnector::init()
	{ 
		msg_receiver_thread = std::make_unique<std::thread>(
				&FrontendConnector::receive_messages, this);
	}

	void FrontendConnector::receive_messages()
	{
		while (!shutting_down) {
			GuiMessage message = channel->message_from(BACKEND);
			switch (message.action) {
			case SET_POSITION:
				handler->set_position(message.pos);
				break;
			}
		}
	}

	void FrontendConnector::play_a_game()
	{
		SPDLOG_INFO("Request to play a game.");
		channel->message_to(BACKEND, {START_GAME});
	}


//StateInfoHandler

	void StateInfoHandler::set_position(const Position& pos)
	{
		state_info.current_pos = pos;
		window->set_position(pos);
	}

}

