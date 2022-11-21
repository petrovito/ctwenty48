#include "search.hh"
#include "types.hh"
#include <algorithm>
#include <boost/random/mersenne_twister.hpp>
#include <oneapi/tbb/parallel_for.h>
#include <rollout_eval.hh>
#include <spdlog/spdlog.h>
#include <vector>

using namespace oneapi::tbb;

namespace c20::search {

	RolloutEvaluator::RolloutEvaluator() :
		gen(boost::random::mt19937(static_cast<std::uint32_t>(std::time(0)))),
		uniform(0, NUM_DIRECTIONS -1)
	{  }


	Value RolloutEvaluator::evaluate(const Position &)
	{
		throw "UNIMPLEMENTED";
	}


	void RolloutEvaluator::batch_evaluate(
			std::vector<search::UserNode*>& nodes, TimeSpan time)
	{
		int discover_time = time *100000;
		int grain = std::max(1, (int)(10000 * nodes.size() / discover_time));

		std::vector<Value> evals(nodes.size());
		std::vector<uint64_t> sum_rollouts(nodes.size());
		std::vector<uint32_t> rollout_counts(nodes.size());
		parallel_for(blocked_range<size_t>(0, nodes.size(), grain),
				[&] (const auto& r) {
			auto popper_copy = *popper;
			auto gen_copy = gen;
			int range_disc_time = r.size() * discover_time / nodes.size();
			/* spdlog::info("{}, {}, {}, {}, ",range_disc_time, r.size(), discover_time, nodes.size()); */

			while (range_disc_time > 0)
				for (auto it = r.begin(); it != r.end(); it++)
				{
					auto move_count = roll_out(nodes[it]->pos, popper_copy, gen_copy);
					sum_rollouts[it] +=  move_count;
					rollout_counts[it]++;
					range_disc_time -= move_count +1;
				}

			for (auto it = r.begin(); it != r.end(); it++)
				evals[it] = ((Value) sum_rollouts[it]) / rollout_counts[it];
		});
		auto max_eval = *std::max_element(evals.begin(), evals.end())
			+ .1; //add small sentinel, so that evals won't equal to 1
		auto min_eval = *std::min_element(evals.begin(), evals.end());
		auto diff = max_eval - min_eval;

		for (int i = 0; i < evals.size(); i++)
		{
			nodes[i]->dist = NodeDistribution::const_dist((evals[i] - min_eval) / diff);
		}
	}

	int RolloutEvaluator::roll_out(const Position& start_pos,
			NumberPopper& popper, boost::mt19937& gen)
	{
		int move_count = 0;
		auto pos = start_pos;
		ZeroIndices zeros;
		while (1) 
		{
			if (pos.is_over()) break;
			//make random move
			while (1)
			{
				int dir_num = uniform(gen);
				MoveDirection random_dir = MoveDirection(dir_num);
				auto result = pos.calc_move(random_dir);
				if (result.has_changed)
				{
					std::tie(pos, zeros) = result.calc_pos_zeros_pair(); 
					break; //inner
				}
			}

			popper.place_one(pos, zeros);
			move_count++;
		}
		return move_count;
	}

	/* Value RolloutEvaluator::rollout_eval(const Position& pos, */ 
	/* 		NumberPopper& popper, boost::mt19937& gen) */
	/* { */
	/* 	std::vector<int> rollout_results; */
	/* 	for (int i = 0; i < num_rollouts; i++) */
	/* 	{ */
	/* 		rollout_results.push_back(roll_out(pos, popper, gen)); */
	/* 	} */
	/* 	return (double)(std::accumulate(rollout_results.begin(), rollout_results.end(), 0)) */
	/* 		/ rollout_results.size(); */
	/* } */
} 

