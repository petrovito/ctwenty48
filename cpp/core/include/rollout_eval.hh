#pragma once

#include "search.hh"
#include "types.hh"
namespace c20::search {

	using namespace commons;

	class RolloutEvaluator : public NodeEvaluator
	{
		private:
			boost::random::uniform_int_distribution<> uniform;
			boost::random::mt19937 gen;

			/* std::array<NumberPopper, NUM_DIRECTIONS> popper_copies; */
			/* std::array<boost::random::mt19937, NUM_DIRECTIONS> gens; */

			NumberPopper *popper;
			int roll_out(const Position& start_pos,
				NumberPopper& popper, boost::mt19937& gen);
			Value rollout_eval(const Position& pos,
					NumberPopper& popper, boost::mt19937& gen);

			template<typename UiEnv> friend class deps::Environment;
		public:
			RolloutEvaluator();
			Value evaluate(const Position&) override;
			void batch_evaluate(std::vector<search::UserNode*>&) override;
	};

}

