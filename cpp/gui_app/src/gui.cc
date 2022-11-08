#include "game_play.hh"
#include "types.hh"
#include <algorithm>
#include <gui.hh>
#include <boost/asio.hpp>

#include <future>
#include <memory>
#include <stdexcept>
#include <thread>

#include <spdlog/spdlog.h>

namespace c20::gui {

//GuiMessageChannel

	void GuiMessageChannel::message_to(AppComponent comp, GuiMessage msg)
	{
		message_q_pair[comp].push(msg);
	}

	bool GuiMessageChannel::message_from(AppComponent comp, GuiMessage& msg)
	{
		return message_q_pair[1-comp].pop(msg);
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
		spdlog::debug("Starting backend receive msgs loop.");
		while (!shutting_down) {
			GuiMessage message;
			channel->message_from(FRONTEND, message);
			spdlog::debug("Message from frontend received. Action {}", message.action);
			switch (message.action) {
			case START_GAME:
				spdlog::info("Frontend request: Start a game.");
				try {
					game_player->start_game();
					game_player->start_bot();
				} catch (std::runtime_error& e) {
					SPDLOG_ERROR(e.what());
				}
				break;
			case EXIT_APP:
				spdlog::info("Frontend request: Exit app.");
				game_player->exit_app();
				break;
			}
		}
		spdlog::debug("Backend receive mgs loop shut down.");
	}

//FrontendConnector


	void FrontendConnector::init()
	{ 
		msg_receiver_thread = std::make_unique<std::thread>(
				&FrontendConnector::receive_messages, this);
	}

	void FrontendConnector::receive_messages()
	{
		spdlog::debug("Starting frontend receive msgs loop.");
		while (!shutting_down) {
			GuiMessage message;
			channel->message_from(BACKEND, message);
			switch (message.action) {
			case SET_POSITION:
				handler->set_position(message.pos);
				break;
			}
		}
		spdlog::debug("Frontend receive mgs loop shut down.");
	}

	void FrontendConnector::exit()
	{
		spdlog::info("Requesting exit");
		channel->message_to(BACKEND, {.action=EXIT_APP});
	}

	void FrontendConnector::play_a_game()
	{
		spdlog::info("Request to play a game.");
		channel->message_to(BACKEND, {.action=START_GAME});
	}


//StateInfoHandler

	void StateInfoHandler::set_position(const Position& pos)
	{
		state_info.current_pos = pos;
		window->set_position(pos);
	}

}

