#pragma once

#include "search.hh"
#include "types.hh"


namespace c20::search {

	class StaticPositionEval : public NodeEvaluator
	{
		public:
			Value evaluate(const Position &) override;
			void batch_evaluate(std::vector<UserNode *> &, TimeSpan) override;
	};

}



