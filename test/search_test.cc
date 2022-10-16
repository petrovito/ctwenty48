#include <algorithm>
#include <gtest/gtest.h>
#include <numeric>
#include <vector>


#include <search.h>
#include <types.hh>

namespace c20::search {

	TEST(SearchTest, FindSubGraph)
	{
		Position pos = Position::from_str("3434|5656|3434|0000");
		GraphSearcher searcher((NumberPopper()));
		auto subgraph = searcher.subgraph_of_depth(pos, 1);
		ASSERT_EQ(1 + 1 + 8, subgraph->nodes.size());
		
		pos = Position::from_str("3434|5656|3434|1560");
		subgraph = searcher.subgraph_of_depth(pos, 2);
		ASSERT_EQ(1 + (1 + 2+0) + (1 + (1+0) + (1+2+4)),
				subgraph->nodes.size());
		auto game_over_nodes = std::count_if(subgraph->nodes.cbegin(),
				subgraph->nodes.cend(), 
				[] (auto&& node) {return node->is_over;});
		ASSERT_EQ(6, game_over_nodes);
		auto final_nodes = std::count_if(subgraph->nodes.cbegin(),
				subgraph->nodes.cend(),
				[] (auto&& node) {return node->is_final;});
		ASSERT_EQ(7, final_nodes);

		pos = Position::from_str("1201|9034|0560|3102");
		subgraph = searcher.subgraph_of_depth(pos, 3);
		ASSERT_LT(30000, subgraph->nodes.size());
	}

}
