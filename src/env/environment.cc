#include "game_logger.hh"
#include "game_play.hh"
#include "selectors.hh"
#include "ui.hh"
#include <env/environment.hh>
#include <memory>


namespace c20::core {

	Environment::Environment() :
		move_selector(new selectors::RandomSelector()),
		ui(new ui::NoopUI()),
		game_player(new GamePlayer(ui, move_selector))
	{}

	void Environment::play_games_and_quit(int num_games, const std::string& path)
	{
		std::unique_ptr<misc::GameLogger> logger(path.empty() ?
				nullptr : new misc::GameLogger(path));
		for (int i = 0; i < num_games; i++)
		{
			auto game = game_player->play_a_game();
			if (logger.get()) logger->log_game(*game);
		}
	}

}

