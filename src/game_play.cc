#include "types.hh"
#include <atomic>
#include <exception>
#include <game_play.hh>
#include <memory>

namespace c20::core {

	StateLocker::StateLocker(std::atomic<State>& _state) :
		state(_state)
	{
		State wait = WAITING;
		if (!state.compare_exchange_weak(wait, RUNNING))
			throw std::exception();
	}

	StateLocker::~StateLocker()
	{
		state.exchange(WAITING);
	}


	GamePlayer::GamePlayer(std::shared_ptr<ui::UIHandler>& _ui,
			   std::shared_ptr<selectors::MoveSelector>& _move_selector) : 
		ui(_ui), move_selector(_move_selector), current_state(WAITING) {}


	void GamePlayer::set_position_for_handlers(const Position& pos)
	{
		ui->set_position(pos);
		move_selector->set_position(pos);
	}
	

	std::unique_ptr<Game> GamePlayer::play_a_game()
	{
		assert(move_selector.get());
		assert(ui.get());
		StateLocker state_lock(current_state);

		current_game = std::unique_ptr<Game>(Game::start_game());
		while (!current_game->is_over())
		{
			set_position_for_handlers(*(current_game->current_position()));
			auto next_move = move_selector->make_move();
			current_game->do_move(next_move);
		}
		return std::move(current_game);
	}


	

}
