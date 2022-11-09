#include "game_play.hh"
#include "types.hh"
#include <algorithm>
#include <cstdlib>
#include <gui.hh>
#include <boost/asio.hpp>

#include <future>
#include <memory>
#include <nana/gui/programming_interface.hpp>
#include <stdexcept>
#include <thread>
#include <chrono>
using namespace std::chrono_literals;

#include <spdlog/spdlog.h>

namespace c20::gui {

//GuiMessageChannel

	void GuiMessageChannel::message_to(AppComponent comp, GuiMessage msg)
	{
		message_q_pair[comp].enqueue(msg);
	}

	bool GuiMessageChannel::message_from(AppComponent comp, GuiMessage& msg)
	{
		return message_q_pair[1-comp].wait_dequeue_timed(msg, .5s);
	}

//BackendConnector

	void BackendConnector::game_over() {}

	void BackendConnector::set_position(const Position& pos) 
	{
		channel->message_to(FRONTEND, {.action=SET_POSITION, .pos=pos});
	}

	void BackendConnector::init()
	{ 
		msg_receiver_thread = 
			new std::thread(&BackendConnector::receive_messages, this);
	}


	void BackendConnector::receive_messages()
	{
		spdlog::debug("Starting backend receive msgs loop.");
		while (!shutting_down) {
			GuiMessage message;
			if (!channel->message_from(FRONTEND, message)) continue;
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

	BackendConnector::~BackendConnector()
	{
		msg_receiver_thread->join();
		delete msg_receiver_thread;
	}

//FrontendConnector


	void FrontendConnector::init()
	{ 
		msg_receiver_thread = 
			new std::thread(&FrontendConnector::receive_messages, this);
	}

	void FrontendConnector::receive_messages()
	{
		spdlog::debug("Starting frontend receive msgs loop.");
		while (!shutting_down) {
			GuiMessage message;
			if (!channel->message_from(BACKEND, message)) continue;
			switch (message.action) {
			case SET_POSITION:
				handler->set_position(message.pos);
				break;
			}
		}
		spdlog::debug("Frontend receive mgs loop shut down.");
	}


	FrontendConnector::~FrontendConnector()
	{
		msg_receiver_thread->join();
	}

	void FrontendConnector::message(const GuiMessage& msg)
	{
		channel->message_to(BACKEND, msg);
	}

//StateInfoHandler

	void StateInfoHandler::set_position(const Position& pos)
	{
		state_info.current_pos.modify(pos);
	}

	void StateInfoHandler::exit()
	{
		spdlog::info("Requesting exit");
		connector->message({.action=EXIT_APP});
	}

	void StateInfoHandler::play_a_game()
	{
		spdlog::info("Request to play a game.");
		connector->message({.action=START_GAME});
	}
}

