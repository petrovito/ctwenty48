#include "cnn.hh"
#include <limits>
#include <utility>


#include <types.hh>
#include <search.hh>
#include <vector>


namespace c20::search {



	UserNode::UserNode(Position pos) : Node(pos), children({}), dist({}) {}
	RandomNode::RandomNode(Position pos) : Node(pos), eval({}) {}
	Node::Node(Position pos) : pos(pos), is_final(false), is_over(false) {}


	GraphSearcher::GraphSearcher(NumberPopper popper) : popper(popper) { }






//NodeDistribution struct


	int NodeDistribution::bin_num_from_val(Value val)
	{
		return (int) (val * BIN_COUNT);
	}


	NodeDistribution NodeDistribution::const_dist(Value eval)
	{
		NodeDistribution dist{.known_ending=0, .bins={}};
		dist.bins[bin_num_from_val(eval)] = 1;
		return dist;
	}

	const NodeDistribution NodeDistribution::const_over_dist = 
		NodeDistribution{.known_ending=1, .bins={}};


//Evaluation struct

	void Evaluation::add_constituent(Probability prob, NodeDistribution& constituent)
	{
		dist.known_ending += prob * constituent.known_ending;
		//TODO use AVX here
		for (int i = 0; i < BIN_COUNT; i++)
		{
			dist.bins[i] += prob * constituent.bins[i]; 
		}
	}

//GraphEvaluator class


	NodeDistribution GraphEvaluator::eval_usernode_recursive(UserNode* user_node)
	{
		//handle terminal nodes
		if (user_node->is_over) //game over node, worst possible evaluation
			return user_node->dist = NodeDistribution::const_over_dist;
		if (user_node->is_final) // final in the given game tree (but not game over)
			return user_node->dist;
				/* NodeDistribution::const_dist(node_eval->evaluate(user_node->pos)); */
		
		//intermediate nodes: need to recurse
		for (auto child_node: user_node->children)
			eval_randomnode_recursive(child_node);
		//find highest evaluation child i.e. best direction
		auto best_child = std::max_element(
				user_node->children.begin(), user_node->children.end(),
				[] (auto node1, auto node2) {
					if (node1 == nullptr) return true;
					if (node2 == nullptr) return false;
					return node1->eval.value < node2->eval.value;
				});
		//set dist and best direction
		user_node->best_dir = MoveDirection(
				std::distance(user_node->children.begin(), best_child));
		return user_node->dist = (*best_child)->eval.dist;
	}


	void GraphEvaluator::eval_randomnode_recursive(RandomNode* random_node)
	{
		if (random_node == nullptr) return;
		random_node->eval = {};
		for (auto [prob, child_node]: random_node->children)
		{
			auto child_eval = eval_usernode_recursive(child_node);
			random_node->eval.add_constituent(prob, child_eval);
		}
		//make evaluation of combined distribution
		eval_and_set(random_node->eval);
	}


	Value GraphEvaluator::eval_and_set(Evaluation& eval)
	{
		//TODO this is expected value, find other ways to evaluate
		eval.value = -1 * eval.dist.known_ending;
		for (int i = 0; i < BIN_COUNT; i++)
		{
			auto bin_prob = eval.dist.bins[i];
			eval.value += bin_prob * i;
		}
		return eval.value;
	}


	void GraphEvaluator::evaluate(GameTree* game_tree)
	{
		eval_usernode_recursive(game_tree->root);
	}

	MoveDirection GraphEvaluator::pick_one(GameTree* game_tree)
	{
		return game_tree->root->best_dir;
	}

#include <iostream>
	using namespace std;
//SearchManager class
	
	SearchManager::SearchManager(NodeEvaluator* _node_eval,
			NumberPopper _popper) : 
		graph_searcher(new GraphSearcher(_popper)), 
		graph_evaluator(new GraphEvaluator()),
		node_eval(_node_eval) {  }

	UserMove SearchManager::make_move()
	{
		int num_zeros = pos.num_zeros();
		int depth = 1;
		if (num_zeros < 12) depth = 2;
		if (num_zeros < 5) depth = 3;
		if (pos.power_sum() > 512) 
		{
			if (num_zeros < 3) depth = 4;
		}

		/* for (int i = 0; i < 16; i++) { */
		/* 	if (i %4 ==0) cout << '|'; */
		/* 	cout <<int(pos[i]); */
		/* } */

		GameTree* tree; 

		final_nodes = std::vector<UserNode*>();
		auto final_node_action = [&] (UserNode* final_node) mutable
			{ final_nodes.push_back(final_node); };

		tree = graph_searcher->subgraph_of_depth(pos, depth, final_node_action);

		/* cout << "   " << tree->nodes.size() << "   " << endl; */
		
		node_eval->batch_evaluate(final_nodes);

		graph_evaluator->evaluate(tree);
		auto dir = graph_evaluator->pick_one(tree);
		return UserMove{dir};
	}




}

