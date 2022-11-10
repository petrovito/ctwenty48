#include "types.hh"
#include <atomic>
#include <boost/asio/post.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/bind/bind.hpp>
#include <exception>
#include <game_play.hh>
#include <memory>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace c20::core {

	void exchange_or_throw(std::atomic<GamePlayerState>& state, GamePlayerState from, GamePlayerState to)
	{
		if (!state.compare_exchange_weak(from, to))
			throw std::runtime_error("Lock in wrong state..");
	};

	StateLocker::StateLocker(std::atomic<GamePlayerState>& _state) :
		state(_state)
	{
		exchange_or_throw(state, IDLE, BOT_ACTIVATED);
	}

	StateLocker::~StateLocker()
	{
		state.exchange(IDLE);
	}

	GamePlayer::GamePlayer() : GamePlayer(nullptr, nullptr) {}

	GamePlayer::GamePlayer(ui::UIHandler* _ui,
			   selectors::MoveSelector* _move_selector) : 
		ui(_ui), move_selector(_move_selector), 
		game_state(IDLE), thread_pool(2) {}


	void GamePlayer::set_position_for_handlers(const Position& pos)
	{
		ui->set_position(pos);
		move_selector->set_position(pos);
	}


	void GamePlayer::exit_app()
	{
		spdlog::info("Shutting down.");
		shutting_down = true;
		stop_game();
	}
	

	std::unique_ptr<Game> GamePlayer::play_a_game()
	{
		spdlog::info("Playing one game.");
		StateLocker state_lock(game_state);

		current_game = std::unique_ptr<Game>(Game::start_game());
		while (!current_game->is_over())
		{
			set_position_for_handlers(*(current_game->current_position()));
			auto next_move = move_selector->make_move();
			current_game->do_move(next_move);
		}
		spdlog::info("Game is over at move {}", current_game->history().size());
		return std::move(current_game);
	}
	

	void GamePlayer::start_game()
	{
		exchange_or_throw(game_state, IDLE, GAME_STARTED);
		spdlog::info("Starting game.");
		current_game = std::unique_ptr<Game>(Game::start_game());
		set_position_for_handlers(*(current_game->current_position()));
	}

	void GamePlayer::stop_game()
	{
		spdlog::info("Stopping game.");
		game_state.exchange(IDLE);
	}


	void GamePlayer::start_bot()
	{
		exchange_or_throw(game_state, GAME_STARTED, BOT_ACTIVATED);
		spdlog::info("Activating bot.");
		boost::asio::post(thread_pool,
				boost::bind(&GamePlayer::bot_loop, this));
	}

	void GamePlayer::stop_bot()
	{
		spdlog::info("Stopping bot.");
		exchange_or_throw(game_state, BOT_ACTIVATED, GAME_STARTED);
	}

	void GamePlayer::bot_loop()
	{
		while (game_state == BOT_ACTIVATED)
		{
			if (current_game->is_over())
			{
				handle_game_over();
				return;
			}
			auto next_move = move_selector->make_move();
			current_game->do_move(next_move);
			set_position_for_handlers(*(current_game->current_position()));
		}
	}

	void GamePlayer::handle_game_over()
	{
		spdlog::info("Game is over at move {}.", current_game->history().size());
		stop_game();
		//TODO notify handlers
	}

	


	

}
