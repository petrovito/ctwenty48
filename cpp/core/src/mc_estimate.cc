#include "types.hh"
#include <algorithm>
#include <iterator>
#include <mc_estimate.hh>
#include <mutex>
#include <numeric>
#include <oneapi/tbb/parallel_for.h>
#include <spdlog/spdlog.h>
#include <vector>
#include "oneapi/tbb.h"

using namespace oneapi::tbb;

namespace c20::search {


	MonteCarloEstimator::MonteCarloEstimator() :
		uniform(0, NUM_DIRECTIONS -1),
		gens{
			boost::random::mt19937(static_cast<std::uint32_t>(std::time(0))),
			boost::random::mt19937(static_cast<std::uint32_t>(std::time(0))),
			boost::random::mt19937(static_cast<std::uint32_t>(std::time(0))),
			boost::random::mt19937(static_cast<std::uint32_t>(std::time(0))),
		}
	{  }


	void MonteCarloEstimator::popper(NumberPopper* popper)
	{
		popper_ = popper;
		for (int i = 0; i < NUM_DIRECTIONS; i++)
			popper_copies[i] = *popper;
	}


	UserMove MonteCarloEstimator::make_move()
	{
		std::array<Value, NUM_DIRECTIONS> evals;
		auto pow_sum = pos_.power_sum();

		parallel_for(blocked_range<size_t>(0, NUM_DIRECTIONS),
				[&] (const auto& r)
		{
			for (auto it = r.begin(); it != r.end(); it++)
			{
				MoveDirection dir = static_cast<MoveDirection>(it);
				auto effect = pos_.calc_move(dir);
				evals[dir] = effect.has_changed ? 
					estimate_eval(effect, dir, pow_sum) : 0;
			}
		});
		auto best_dir = MoveDirection(std::distance(evals.begin(), 
				std::max_element(evals.begin(), evals.end())));
		return UserMove{.direction=best_dir};		
	}



	Value MonteCarloEstimator::estimate_eval(MoveResultSet& effect, 
			MoveDirection dir, int pow_sum)
	{
		std::vector<int> rollout_results;
		int num_rollouts = 250;
		if (pow_sum > 500) num_rollouts = 500;
		if (pow_sum > 1000) num_rollouts = 1200;
		if (pow_sum > 1500) num_rollouts = 2000;
		if (pow_sum > 2000) num_rollouts = 3000;
		if (pow_sum > 2500) num_rollouts = 5000;
		for (int i = 0; i < num_rollouts; i++)
		{
			rollout_results.push_back(roll_out(effect, dir));
		}
		return (double)(std::accumulate(rollout_results.begin(), rollout_results.end(), 0))
			/ rollout_results.size();
	}


	int MonteCarloEstimator::roll_out(MoveResultSet& start_pos, MoveDirection dir)
	{
		auto [pos, zeros] = start_pos.calc_pos_zeros_pair();
		int move_count = 0;
		while (1) 
		{
			popper_copies[dir].place_one(pos, zeros);
			if (pos.is_over()) break;
			//make random move
			while (1)
			{
				int dir_num = uniform(gens[dir]);
				MoveDirection random_dir = MoveDirection(dir_num);
				auto result = pos.calc_move(random_dir);
				if (result.has_changed)
				{
					std::tie(pos, zeros) = result.calc_pos_zeros_pair(); 
					break; //inner
				}
			}
			move_count++;
		}
		return move_count;
	}


	Analysis MonteCarloEstimator::analyze(const Position& pos)
	{
		std::lock_guard<std::mutex> lock(analyze_mutex);
		std::array<Value, NUM_DIRECTIONS> evals;
		auto pow_sum = pos.power_sum();
		for (auto dir: directions)
		{
			auto effect = pos.calc_move(dir);
			evals[dir] = effect.has_changed ? estimate_eval(effect, dir, pow_sum) : 0;
		}
		auto best_dir_value = *std::max_element(evals.begin(), evals.end());
		//TODO above is just copy paste code
		return Analysis{.position_val=best_dir_value, .deep_values{evals}};
	}

}

