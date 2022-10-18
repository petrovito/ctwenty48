#pragma once
#include "game_play.hh"
#include <types.hh>

namespace c20::core {

	/**
	 * Class responsible for instantiating components, and wire links.
	 * Basically, entry point into application.
	 */
	class Environment
	{
		private:
			std::shared_ptr<MoveSelector> move_selector;
			std::shared_ptr<UIHandler> ui;
			std::shared_ptr<GamePlayer> game_player;
		public:
			Environment();
			void play_games_and_quit(int num_games, const std::string& path);
	};

}

