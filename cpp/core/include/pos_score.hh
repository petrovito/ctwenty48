#pragma once

#include "mcts.hh"
#include "search.hh"
#include "types.hh"
#include "gtest/gtest_prod.h"


namespace c20::search {


	struct path_attr
	{
		int len;
		int breaker_num;
		int optimal_num; //what should be there instead of breaker
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



