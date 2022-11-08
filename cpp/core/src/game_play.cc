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

	void lock_or_throw(std::atomic<State>& state)
	{
		State wait = WAITING;
		if (!state.compare_exchange_weak(wait, RUNNING))
			throw std::runtime_error("Lock is already taken.");
	};

	StateLocker::StateLocker(std::atomic<State>& _state) :
		state(_state)
	{
		lock_or_throw(state);
	}

	StateLocker::~StateLocker()
	{
		state.exchange(WAITING);
	}

	GamePlayer::GamePlayer() : GamePlayer(nullptr, nullptr) {}

	GamePlayer::GamePlayer(ui::UIHandler* _ui,
			   selectors::MoveSelector* _move_selector) : 
		ui(_ui), move_selector(_move_selector), 
		game_state(WAITING), thread_pool(2) {}


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
		lock_or_throw(game_state);
		spdlog::info("Starting game.");
		current_game = std::unique_ptr<Game>(Game::start_game());
		set_position_for_handlers(*(current_game->current_position()));
	}

	void GamePlayer::stop_game()
	{
		//TODO race conditions here..
		spdlog::info("Stopping game.");
		bot_state.exchange(WAITING);
		game_state.exchange(WAITING);
	}


	void GamePlayer::start_bot()
	{
		//TODO resolve race conditions here
		if (game_state != RUNNING) 
			throw std::runtime_error("Game is not running");
		lock_or_throw(bot_state);
		spdlog::info("Activating bot.");
		boost::asio::post(thread_pool,
				boost::bind(&GamePlayer::bot_loop, this));
	}

	void GamePlayer::stop_bot()
	{
		spdlog::info("Starting bot.");
		bot_state.exchange(WAITING);
	}

	void GamePlayer::bot_loop()
	{
		while (bot_state == RUNNING)
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
