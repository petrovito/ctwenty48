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
		NodeContainer* node_container = new NodeContainer();
		NumberPopper popper;
		GraphSearcher searcher(&popper, node_container);
		
		Position pos = Position::from_str("3434|5656|3434|0000");
		node_container->reset(pos);
		int new_nodes = searcher.search_level();
		ASSERT_EQ(8, new_nodes);
		
		pos = Position::from_str("3434|5656|3434|1560");
		node_container->reset(pos);
		new_nodes = searcher.search_level();
		ASSERT_EQ(4, new_nodes);

		new_nodes = searcher.search_level();
		ASSERT_EQ(4, new_nodes);

		auto final_nodes = node_container->get_final_nodes();
		ASSERT_EQ(7, final_nodes.size());
		auto game_over_nodes_count = std::count_if(final_nodes.begin(),
				final_nodes.end(),
				[] (auto node) {return node->is_over;});
		ASSERT_EQ(6, game_over_nodes_count);

		/* pos = Position::from_str("1201|9034|0560|3102"); */
		/* subgraph = searcher.subgraph_of_depth(pos, 3, noop); */
		/* ASSERT_LT(30000, subgraph->nodes.size()); */
	}



#define ADD_NODE(TYPE, NAME, POS) pos = Position::from_str("1234|1234|1234|123" #POS); \
	auto NAME = \
	node_container->push_##TYPE(pos); \
	

	TEST(SearchTest, EvaluateGameTree)
	{
		Value val_r11 = .4,
			  val_r12 = .7,
			  val_r21 = .3;
	
		auto node_container = new NodeContainer();

		Position pos = Position::from_str("1234|1234|1234|1231");
		node_container->reset(pos);
		auto root = node_container->root_node();

		/* ADD_NODE(usernode, root, 1); */
			ADD_NODE(randomnode, r1, 2); root->children.at(LEFT)=r1;
				ADD_NODE(usernode, r11, 3); r1->children.emplace_back(.4, r11);
					r11->is_final=true;
				ADD_NODE(usernode, r12, 4); r1->children.emplace_back(.6, r12);
					r12->is_final=true;
			ADD_NODE(randomnode, r2, 5); root->children.at(UP)=r2;
				ADD_NODE(usernode, r21, 6); r2->children.emplace_back(1., r21);
					r21->is_final=true;

		r11->dist = NodeDistribution::const_dist(val_r11);
		r12->dist = NodeDistribution::const_dist(val_r12);
		r21->dist = NodeDistribution::const_dist(val_r21);

		GraphEvaluator graph_evaluator;
		graph_evaluator.evaluate(node_container); 

		ASSERT_NEAR(int(val_r21*BIN_COUNT), r2->eval.value, 1e-5);
		ASSERT_NEAR(.4*int(val_r11*BIN_COUNT) + .6*int(val_r12*BIN_COUNT), 
				r1->eval.value, 1e-5);
		ASSERT_EQ(LEFT, root->best_dir);
		ASSERT_NEAR(.4, root->dist.bins[int(val_r11*BIN_COUNT)], 1e-5);
		ASSERT_NEAR(.6, root->dist.bins[int(val_r12*BIN_COUNT)], 1e-5);
		ASSERT_NEAR(.0, root->dist.bins[int(.6*BIN_COUNT)], 1e-5);


		//change a node to game over node.
		r12->is_over = true;
		graph_evaluator.evaluate(node_container);

		ASSERT_NEAR(.4*int(val_r11*BIN_COUNT) + .6*int(-1)/*i.e. known ending*/,
				r1->eval.value, 1e-5);
		ASSERT_EQ(UP, root->best_dir); //we have a new best dir
	}
}

