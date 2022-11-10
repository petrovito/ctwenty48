#include "game_play.hh"
#include "types.hh"
#include <algorithm>
#include <cstddef>
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


	boost::mt19937_64 key_gen(static_cast<std::uint32_t>(std::time(0)));
	
	message_key generate_key()
	{
		return static_cast<message_key>(key_gen());
	}

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
					channel->message_to(FRONTEND, 
							{.key=message.key, .is_response=true, .status=OK});
				} catch (std::runtime_error& e) {
					SPDLOG_ERROR(e.what());
					channel->message_to(FRONTEND, 
							{.key=message.key, .is_response=true, .status=FAILURE});
				}
				break;
			case ACTIVATE_BOT:
				spdlog::info("Frontend request: Start bot.");
				try {
					game_player->start_bot();
					channel->message_to(FRONTEND, 
							{.key=message.key, .is_response=true, .status=OK});
				} catch (std::runtime_error& e) {
					SPDLOG_ERROR(e.what());
					channel->message_to(FRONTEND, 
							{.key=message.key, .is_response=true, .status=FAILURE});
				}
				break;
			case STOP_BOT:
				spdlog::info("Frontend request: Start bot.");
				try {
					game_player->stop_bot();
					channel->message_to(FRONTEND, 
							{.key=message.key, .is_response=true, .status=OK});
				} catch (std::runtime_error& e) {
					SPDLOG_ERROR(e.what());
					channel->message_to(FRONTEND, 
							{.key=message.key, .is_response=true, .status=FAILURE});
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
			if (message.is_response) {
				if (sent_requests.count(message.key)) {
					GuiMessage request = sent_requests[message.key];
					sent_requests.erase(message.key);
					switch (request.action) {
					case START_GAME:
						if (message.status == OK) {
							spdlog::debug("Game has started.");
							stateinfo_handler->game_state_changed(core::GAME_STARTED);
						} else spdlog::error("Game couldn't be started.");
						break;
					case STOP_BOT:
						if (message.status == OK) {
							spdlog::debug("Bot has stopped.");
							stateinfo_handler->game_state_changed(core::GAME_STARTED);
						} else spdlog::error("Game couldn't be started.");
						break;
					case ACTIVATE_BOT:
						if (message.status == OK) {
							spdlog::debug("Bot has started.");
							stateinfo_handler->game_state_changed(core::BOT_ACTIVATED);
						} else spdlog::error("Game couldn't be started.");
						break;
					}					
				} else {
					spdlog::error("No request found with response msg key.");
				}
				continue;
			}
			switch (message.action) {
			case SET_POSITION:
				stateinfo_handler->set_position(message.pos);
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

	void FrontendConnector::request(GuiMessage&& msg)
	{
		auto key = generate_key();
		msg.key = key;
		sent_requests[key] = msg;
		channel->message_to(BACKEND, msg);
	}

//StateInfoHandler

	void StateInfoHandler::set_position(const Position& pos)
	{
		state_info.table_pos.modify(pos);
		state_info.history_view.get().push_pos_and_adjust(pos);
		state_info.history_view.notify();
	}

	void StateInfoHandler::game_state_changed(const core::GamePlayerState& state)
	{
		state_info.game_state.modify(state);
	}


	void StateInfoHandler::move_history_view(int key) {
		auto& history_view = state_info.history_view.get();
		int history_idx = history_view.current_idx();
		bool changed = false;
		switch (key) {
			case 37: //LEFT
				if (history_idx >= history_view.height()) {
					history_view.adjust(history_idx -history_view.height());
					changed = true;
				}
				break;
			case 39: //RIGHT
				if (history_idx < history_view.size() - history_view.height()) {
					history_view.adjust(history_idx +history_view.height());
					changed = true;
				}
				break;
			case 40: //DOWN
				if (history_idx < history_view.size() -1) {
					history_view.adjust(history_idx +1);
					changed = true;
				}
				break;
			case 38: //UP
				if (history_idx >= 1) {
					history_view.adjust(history_idx -1);
					changed = true;
				}
				break;
		}
		if (changed)
		{
			state_info.history_view.notify();
			state_info.table_pos.modify(history_view.at(
						history_view.current_idx()).pos);
		}
	}

	void StateInfoHandler::exit()
	{
		spdlog::info("Requesting exit");
		connector->message({.action=EXIT_APP});
	}

	void StateInfoHandler::start_game()
	{
		spdlog::info("Request to play a game.");
		connector->request({.action=START_GAME});
	}

	void StateInfoHandler::change_bot_state()
	{
		switch (state_info.game_state.get()) {
			case core::IDLE:
				throw std::runtime_error("Game has not started.");
			case core::GAME_STARTED:
				spdlog::info("Request to start bot");
				connector->request({.action=ACTIVATE_BOT});
				break;
			case core::BOT_ACTIVATED:
				spdlog::info("Request to stop bot");
				connector->request({.action=STOP_BOT});
				break;
		}
	}
}

