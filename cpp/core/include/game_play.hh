#pragma once

#include <atomic>
#include <boost/asio/thread_pool.hpp>
#include <memory>
#include <types.hh>
#include <ui.hh>
#include <selectors.hh>

namespace c20::core {

	using namespace commons;

	enum State 
	{
		WAITING,
		RUNNING,
	};


	class GamePlayer 
	{
		private:
			selectors::MoveSelector* move_selector;
			ui::UIHandler* ui;

			std::unique_ptr<Game> current_game;
			std::atomic<State> game_state;
			std::atomic<State> bot_state;

			boost::asio::thread_pool thread_pool;

			void set_position_for_handlers(const Position&);
			void handle_game_over();
			void bot_loop();

			template<typename UiEnv> friend class deps::Environment;
		public:
			GamePlayer();
			GamePlayer(ui::UIHandler*, selectors::MoveSelector*);
			/** Dumb stateless play one game method. */
			std::unique_ptr<Game> play_a_game();

			void start_game();
			void stop_game();
			void start_bot();
			void stop_bot();

			void exit_app();

	};

	class StateLocker
	{
		private:
			std::atomic<State>& state;
		public:
			StateLocker(std::atomic<State>&);
			~StateLocker();
	};


}
