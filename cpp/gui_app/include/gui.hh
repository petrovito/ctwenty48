#pragma once

#include <blockingconcurrentqueue.h>
#include "frontend_game.hh"
#include "observable.hh"
#include "types.hh"
#include "widgets.hh"
#include <memory>
#include <game_play.hh>
#include <sys/types.h>
#include <thread>
#include <ui.hh>
#include <boost/asio/thread_pool.hpp>
#include <unordered_map>

namespace c20::deps { class GuiEnv; }

namespace c20::gui {
	using namespace commons;

	enum GuiAction 
	{
		//from frontend
		START_GAME,
		ACTIVATE_BOT,
		STOP_BOT,
		ANALYZE,
		STOP_ANALYZE,
		EXIT_APP,

		//from backend
		SET_POSITION
	};


	enum Status
	{
		OK,
		FAILURE,
	};


	struct GuiMessage
	{
		message_key key = 0;
		bool is_response = false;

		GuiAction action;

		Status status;

		Position pos;

		Analysis analysis;
	};


	typedef moodycamel::BlockingConcurrentQueue<GuiMessage> message_q;

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
			std::thread* msg_receiver_thread;

			void receive_messages();
			void init();

			friend class deps::GuiEnv;
		public:
			BackendConnector() = default;
			virtual ~BackendConnector();

			virtual void set_position(const Position&);
	 		virtual void game_over();
			virtual void analysis_msg(const Analysis&, message_key);
	};


	/**
	 *  Class that represents the full set of information
	 */
	struct StateInfo
	{
		Observable<Position> table_pos;
		Observable<GameHistoryView<>> history_view;
		Observable<core::GamePlayerState> game_state;
		Observable<Analysis> analysis; //TODO this seems a bit iffy..
	};


	class FrontendConnector;


	class StateInfoHandler
	{
		private:
			C2048Window* window;
			FrontendConnector* connector;


			friend class deps::GuiEnv;
		public:
			StateInfo state_info;

			void set_position(const Position&);
			void game_state_changed(const core::GamePlayerState& state);
			void analysis(const Analysis&);

			void move_history_view(int key);
			void start_game();
			void change_bot_state();
			void change_analyze_state();
			void request_analysis(const Position&);
			void exit();
	};


	/**
	 *  Frontend component that is responsible for communicating
	 *  with backend through the GuiMessageChannel.
	 */
	class FrontendConnector
	{
		private:
			GuiMessageChannel* channel;
			StateInfoHandler* stateinfo_handler;
			std::thread* msg_receiver_thread;

			std::unordered_map<message_key, GuiMessage> sent_requests;
			
			void receive_messages();
			void init();

			friend class deps::GuiEnv;
		public:
			FrontendConnector() = default;
			virtual ~FrontendConnector();
			void message(const GuiMessage&);
			void request(GuiMessage&&);
	};


}
