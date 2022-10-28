#pragma once
#include "game_play.hh"
#include <memory>
#include <types.hh>

namespace c20::gui { class C2048Window; }

namespace c20::core {

	/**
	 * Class responsible for instantiating components, and wire links.
	 * Basically, entry point into application.
	 */
	class Environment
	{
		public:
			std::shared_ptr<MoveSelector> move_selector;
			std::shared_ptr<ui::UIHandler> ui;
			std::shared_ptr<GamePlayer> game_player;
			std::shared_ptr<gui::C2048Window> window;
		public:
			//Environment();
			void play_games_and_quit(int num_games, const std::string& path);
	};

}

