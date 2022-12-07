#pragma once

#include "search.hh"
#include "types.hh"
#include "gtest/gtest_prod.h"
#include <boost/container/static_vector.hpp>
#include <cstdint>


namespace c20::search {

	using boost::container::static_vector;

	struct path_attr
	{
		uint8_t len;
		uint8_t breaker_num;
		uint8_t optimal_num; //what should be there instead of breaker
		static_vector<uint8_t, 4> diffs;
	};
	
	struct MctsHyperParams
	{
		//mcts ops
		double const_C_mult = std::sqrt(2);
		double decline_pow = .75;
		double rollout_corner_weight = 2.;
		//scoring
		std::vector<double> main_path_mults{1,2,4,8,26,40};
		std::vector<double> path_diff_mults{2,2,2,2};
		double path_diff_pow = 2.;
		double breaker_mult = 2.7;
		double breaker_pow = 2.;
	};
	

	class StaticPositionEval : public NodeEvaluator
	{
		private:
			MctsHyperParams params;

			template<typename UiEnv> friend class deps::Environment;
			FRIEND_TEST(PosScoreTest, MainPath);
		public:
			path_attr find_main_path(const Position&);
			Value evaluate(const Position &) override;
			void batch_evaluate(std::vector<UserNode *> &, TimeSpan) override;
	};

}



