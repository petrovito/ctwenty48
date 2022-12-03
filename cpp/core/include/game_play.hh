#pragma once

#include <atomic>
#include <boost/asio/thread_pool.hpp>
#include <memory>
#include <types.hh>
#include <ui.hh>
#include <selectors.hh>

namespace c20::core {

	using namespace commons;

	enum GamePlayerState 
	{
		IDLE,
		GAME_STARTED,
		BOT_ACTIVATED,
		ANALYZING,
	};


	class GamePlayer 
	{
		private:
			ui::UIHandler* ui;
			selectors::MoveSelector* move_selector;

			std::unique_ptr<Game> current_game;
			std::atomic<GamePlayerState> game_state;

			boost::asio::thread_pool thread_pool;

			void set_position_for_handlers(const Position&);
			void handle_game_over();
			void bot_loop();

			void analyze(const Position&, message_key);

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

			void analyze_async(const Position&, message_key);

			void exit_app();
	};

	class StateLocker
	{
		private:
			std::atomic<GamePlayerState>& state;
		public:
			StateLocker(std::atomic<GamePlayerState>&);
			~StateLocker();
	};


}
