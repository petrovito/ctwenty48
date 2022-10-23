#include "cnn.hh"
#include <limits>
#include <utility>


#include <types.hh>
#include <search.hh>


namespace c20::search {



	UserNode::UserNode(Position pos) : Node(pos), children({}), dist({}) {}
	RandomNode::RandomNode(Position pos) : Node(pos), eval({}) {}
	Node::Node(Position pos) : pos(pos), is_final(false), is_over(false) {}


	GraphSearcher::GraphSearcher(NumberPopper popper) : popper(popper) { }



	GameTree* GraphSearcher::subgraph_of_depth(Position& pos, int depth)
	{
		current_tree = GameTree();
		current_tree.root = user_node_recursive(pos, depth);
		return &current_tree;
	}



	UserNode* GraphSearcher::user_node_recursive(Position pos, int depth) 
	{
		auto start_node = new UserNode(pos);
		current_tree.nodes.emplace_back(start_node);
		if (pos.is_over()) //special case of game_over nodes
		{
			start_node->is_over = true;
			start_node->is_final = true;
			return start_node;
		}
		if (depth == 0) // stop recursion at depth 1
		{
			start_node->is_final = true;
			return start_node;
		}
		for (auto dir: directions)
		{
			auto effect = pos.calc_move(dir);
			if (effect.has_changed) 
			{
				auto [new_pos, zeros] = 
					effect.calc_pos_zeros_pair();
				start_node->children[dir] = 
					random_node_recursive(new_pos, zeros, depth);
			} 
			else //illegal direction 
			{
				start_node->children[dir] = nullptr;
			}
		}
		return start_node;
	}


	RandomNode* GraphSearcher::random_node_recursive(
			Position pos, ZeroIndices& zeros, int depth)
	{
		auto random_node = new RandomNode(pos);
		current_tree.nodes.emplace_back(random_node);
		for (auto [prob, child_pos]: popper.dist_from(pos, zeros))
		{
			random_node->children.push_back({prob,
					user_node_recursive(child_pos, depth -1)});
		}
		return random_node;
	}



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

	GraphEvaluator::GraphEvaluator(NodeEvaluator* _node_eval) :
		node_eval(_node_eval) {}


	NodeDistribution GraphEvaluator::eval_usernode_recursive(UserNode* user_node)
	{
		//handle terminal nodes
		if (user_node->is_over) //game over node, worst possible evaluation
			return user_node->dist = NodeDistribution::const_over_dist;
		if (user_node->is_final) // final in the given game tree (but not game over)
			return user_node->dist =
				NodeDistribution::const_dist(node_eval->evaluate(user_node->pos));
		
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
		graph_evaluator(new GraphEvaluator(_node_eval)),
		node_eval(_node_eval) {  }

	UserMove SearchManager::make_move()
	{
		int num_zeros = pos.num_zeros();
		int depth = 1;
		if (num_zeros < 8) depth = 2;
		if (num_zeros < 4) depth = 3;
		if (num_zeros < 2) depth = 4;

		for (int i = 0; i < 16; i++) {
			if (i %4 ==0) cout << '|';
			cout <<int(pos[i]);
		}

		auto tree = graph_searcher->subgraph_of_depth(pos, depth);
		graph_evaluator->evaluate(tree);
		auto dir = graph_evaluator->pick_one(tree);
		cout << "   " << tree->nodes.size();
		cout << endl;
		return UserMove{dir};
	}




}

