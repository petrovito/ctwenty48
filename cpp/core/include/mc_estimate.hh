#pragma once

#include "selectors.hh"
#include <mutex>
#include <types.hh>


namespace  c20::search {

	using namespace commons;


	class MonteCarloEstimator : public selectors::MoveSelector
	{
		private:
			boost::random::uniform_int_distribution<> uniform;
			NumberPopper *popper_;

			std::array<NumberPopper, NUM_DIRECTIONS> popper_copies;
			std::array<boost::random::mt19937, NUM_DIRECTIONS> gens;

			std::mutex analyze_mutex;

			Value estimate_eval(MoveResultSet&, MoveDirection, int);
			int roll_out(MoveResultSet&, MoveDirection);

			template<typename UiEnv> friend class deps::Environment;
		public:
			MonteCarloEstimator();
			void popper(NumberPopper*);

			UserMove make_move() override;
			Analysis analyze(const Position &) override;

	};



}

