#pragma once

#include "oneapi/tbb/concurrent_queue.h"
#include "types.hh"
#include "widgets.hh"
#include <memory>
#include <game_play.hh>
#include <thread>
#include <ui.hh>
#include <boost/asio/thread_pool.hpp>

namespace c20::deps { class GuiEnv; }

namespace c20::gui {
	using namespace commons;

	enum GuiAction 
	{
		//from frontend
		START_GAME,
		EXIT_APP,

		//from backend
		SET_POSITION
	};

	typedef long long int message_key;

	enum Status
	{
		OK,
		FAILURE,
	};

	struct GuiMessage
	{
		message_key key;
		GuiAction action;

		Position pos;
	};

	struct GuiResponse
	{
		message_key key;
		Status status;
	};

	struct MessagePair
	{
		GuiMessage request;
		GuiResponse answer;
	};

	typedef tbb::concurrent_bounded_queue<GuiMessage> message_q;

	enum AppComponent
	{
		FRONTEND = 0,
		BACKEND = 1,
	};

	/**
	 *  Simple wrapper class around a concurrent queue-pair
	 *  that represents communication channels between the 
	 *  fronted and backend.
	 */
	class GuiMessageChannel
	{
		private:
			std::array<message_q, 2> message_q_pair;
		public:
			GuiMessageChannel() = default;
			void message_to(AppComponent, GuiMessage msg);
			/**
			 *  Read a message from the specified app component.
			 *  Blocks until a message is available.
			 */
			bool message_from(AppComponent, GuiMessage&);
	};

	/**
	 *  Backend component that is responsible for communicating
	 *  with frontend through the GuiMessageChannel.
	 */
	class BackendConnector : public ui::UIHandler
	{
		private:
			GuiMessageChannel* channel;
			std::unique_ptr<std::thread> msg_receiver_thread;

			void receive_messages();
			void init();

			friend class deps::GuiEnv;
		public:
			BackendConnector() = default;

			virtual void set_position(const Position&);
			virtual void game_over();

	};


	/**
	 *  Class that represents the full set of information
	 */
	struct StateInfo
	{
		Position current_pos;
	};


	class FrontendConnector;


	class StateInfoHandler
	{
		private:
			C2048Window* window;
			FrontendConnector* connector;

			StateInfo state_info;

			friend class deps::GuiEnv;
		public:
			void set_position(const Position&);
	};


	/**
	 *  Frontend component that is responsible for communicating
	 *  with backend through the GuiMessageChannel.
	 */
	class FrontendConnector
	{
		private:
			GuiMessageChannel* channel;
			StateInfoHandler* handler;
			std::unique_ptr<std::thread> msg_receiver_thread;
			
			void receive_messages();
			void init();

			friend class deps::GuiEnv;
		public:
			FrontendConnector() = default;
			void play_a_game();
			void exit();
	};


}
