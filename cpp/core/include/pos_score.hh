#pragma once

#include "mcts.hh"
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

	class StaticPositionEval : public NodeEvaluator
	{
		private:
			path_attr find_main_path(const Position&);
			mcts::MctsHyperParams params;

			template<typename UiEnv> friend class deps::Environment;
			FRIEND_TEST(PosScoreTest, MainPath);
		public:
			Value evaluate(const Position &) override;
			void batch_evaluate(std::vector<UserNode *> &, TimeSpan) override;
	};

}



