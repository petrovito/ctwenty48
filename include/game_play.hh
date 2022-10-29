#pragma once

#include <atomic>
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
			std::unique_ptr<Game> current_game;
			std::shared_ptr<selectors::MoveSelector> move_selector;
			std::shared_ptr<ui::UIHandler> ui;
			std::atomic<State> current_state;

			void set_position_for_handlers(const Position&);
		public:
			GamePlayer(std::shared_ptr<ui::UIHandler>&,
					std::shared_ptr<selectors::MoveSelector>&);
			/** Dumb stateless play one game method. */
			std::unique_ptr<Game> play_a_game();
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
