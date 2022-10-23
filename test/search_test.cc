#include <algorithm>
#include <exception>
#include <gtest/gtest.h>
#include <numeric>
#include <stdexcept>
#include <vector>

#include <types.hh>
#include <search.hh>

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

#define ADD_NODE(TYPE, NAME, POS) TYPE* NAME = \
	new TYPE(Position::from_str("1234|1234|1234|123" #POS)); \
	game_tree.nodes.emplace_back(NAME)


	class TestEvaluator : public NodeEvaluator {
		virtual Value evaluate(Position& pos) {
			switch (pos[NUM_SQUARES -1]) {
				case 3: return .4;
				case 4: return .7;
				case 6: return .3;
			}
			throw std::runtime_error("unknown value");
		}
	};

	TEST(SearchTest, EvaluateGameTree)
	{
		
			GameTree game_tree;
			ADD_NODE(UserNode, root, 1);
				ADD_NODE(RandomNode, r1, 2); root->children.at(LEFT)=r1;
					ADD_NODE(UserNode, r11, 3); r1->children.emplace_back(.4, r11);
						r11->is_final=true;
					ADD_NODE(UserNode, r12, 4); r1->children.emplace_back(.6, r12);
						r12->is_final=true;
				ADD_NODE(RandomNode, r2, 5); root->children.at(UP)=r2;
					ADD_NODE(UserNode, r21, 6); r2->children.emplace_back(1., r21);
						r21->is_final=true;

			game_tree.root = root;

			TestEvaluator evaluator;
			GraphEvaluator graph_evaluator(&evaluator);
			graph_evaluator.evaluate(&game_tree); 

			ASSERT_NEAR(int(.3*BIN_COUNT), r2->eval.value, 1e-5);
			ASSERT_NEAR(.4*int(.4*BIN_COUNT) + .6*int(.7*BIN_COUNT), r1->eval.value, 1e-5);
			ASSERT_EQ(LEFT, root->best_dir);
			ASSERT_NEAR(.4, root->dist.bins[int(.4*BIN_COUNT)], 1e-5);
			ASSERT_NEAR(.6, root->dist.bins[int(.7*BIN_COUNT)], 1e-5);
			ASSERT_NEAR(.0, root->dist.bins[int(.6*BIN_COUNT)], 1e-5);


			//change a node to game over node.
			r12->is_over = true;
			graph_evaluator.evaluate(&game_tree);

			ASSERT_NEAR(.4*int(.4*BIN_COUNT) + .6*int(-1)/*i.e. known ending*/,
					r1->eval.value, 1e-5);
			ASSERT_EQ(UP, root->best_dir); //we have a new best dir
		
	}

}
