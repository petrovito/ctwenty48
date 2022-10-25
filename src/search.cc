#include "cnn.hh"
#include <iterator>
#include <limits>
#include <stdexcept>
#include <utility>


#include <types.hh>
#include <search.hh>
#include <vector>


namespace c20::search {



	UserNode::UserNode(Position pos) : Node(pos), children({}), dist({}) {}
	RandomNode::RandomNode(Position pos) : Node(pos), eval({}) {}
	Node::Node(Position pos) : pos(pos), is_final(false), is_over(false) {}

	UserNode::UserNode() : Node(), children({}), dist({}) {}
	RandomNode::RandomNode() : Node(), children({}), eval({}) {}
	Node::Node() : pos({}), is_final(false), is_over(false) {}


	GraphSearcher::GraphSearcher(NumberPopper& _popper, NodeContainer* nc) :
		popper(_popper), node_container(nc) { }






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


	void GraphEvaluator::evaluate(NodeContainer* game_tree)
	{
		eval_usernode_recursive(game_tree->root_node());
	}

	MoveDirection GraphEvaluator::pick_one(NodeContainer* game_tree)
	{
		return game_tree->root_node()->best_dir;
	}

#include <iostream>
	using namespace std;
//SearchManager class
	
	SearchManager::SearchManager(NodeEvaluator* _node_eval,
			NumberPopper _popper) : 
		node_container(new NodeContainer()),
		graph_searcher(new GraphSearcher(_popper, node_container)), 
		graph_evaluator(new GraphEvaluator()),
		node_eval(_node_eval) {  }

	UserMove SearchManager::make_move()
	{
		/* for (int i = 0; i < 16; i++) { */
		/* 	if (i %4 ==0) cout << '|'; */
		/* 	cout <<int(pos[i]); */
		/* } */

		node_container->reset(pos);

		while (node_container->usernode_count() < 10000) {
			int new_nodes = graph_searcher->search_level();
			if (new_nodes == 0) break;
		}

		/* cout << "   " << tree->nodes.size() << "   " << endl; */
		
		auto final_nodes = node_container->get_final_nodes();
		node_eval->batch_evaluate(final_nodes);

		graph_evaluator->evaluate(node_container);
		auto dir = graph_evaluator->pick_one(node_container);
		return UserMove{dir};
	}

//NodeContainer class

	NodeContainer::NodeContainer() : 
		usernode_buf(new UserNode[NUM_USERNODES]),
		randomnode_buf(new RandomNode[NUM_RANDOMNODES]),
		root(usernode_buf)
	{}


	UserNode* NodeContainer::push_usernode(Position& pos)
	{
		if (usernode_idx == NUM_USERNODES)
			throw std::runtime_error("Index out of bound.");
		usernode_buf[usernode_idx] = UserNode(pos);
		auto ptr = &usernode_buf[usernode_idx];
		usernode_idx++;
		return ptr;
	}


	RandomNode* NodeContainer::push_randomnode(Position& pos)
	{
		if (randomnode_idx == NUM_RANDOMNODES)
			throw std::runtime_error("Index out of bound.");
		randomnode_buf[randomnode_idx] = RandomNode(pos);
		auto ptr = &randomnode_buf[randomnode_idx];
		randomnode_idx++;
		return ptr;
	}


	void NodeContainer::reset(Position& pos)
	{
		current_level = 0;
		usernode_levels[0] = 1;
		randomnode_levels[0] = 0;
		usernode_buf[0] = UserNode(pos);
		usernode_idx = 1;
		randomnode_idx = 0;
	}

	void NodeContainer::increase_level()
	{
		current_level++;
		usernode_levels[current_level] = usernode_idx;
		randomnode_levels[current_level] = randomnode_idx;
	}

	int NodeContainer::last_level_length() 
	{
		return usernode_levels[current_level] -
			usernode_levels[current_level -1];
	}

	std::vector<UserNode*> NodeContainer::get_final_nodes()
	{
		std::vector<UserNode*> final_nodes;
		UserNode* end = usernode_buf + usernode_idx;
		for (auto node = usernode_buf; node != end; node++)
		{
			if (node->is_final) final_nodes.push_back(node);
		}
		return final_nodes;
	}

	std::pair<UserNode*, UserNode*> NodeContainer::last_level_usernodes()
	{
		return {usernode_buf + usernode_levels[current_level -1], 
				usernode_buf + usernode_levels[current_level]};
	}


//GraphSearcher class

	RandomNode* GraphSearcher::random_node(Position pos, ZeroIndices& zeros)
	{
		auto random_node = node_container->push_randomnode(pos);
		for (auto [prob, child_pos]: popper.dist_from(pos, zeros))
		{
			auto child_node = node_container->push_usernode(child_pos);
			random_node->children.push_back({prob, child_node});
			child_node->is_over = child_pos.is_over();
			//marking it final, potentially unmark in next iteration
			child_node->is_final = true; 
		}
		return random_node;
	}


	int GraphSearcher::search_level() 
	{
		int new_usernodes = 0;
		auto [start, end] = node_container->last_level_usernodes();
		for (UserNode* user_node = start; user_node != end; user_node++)
		{
			if (user_node->is_over) continue;
			user_node->is_final = false;
			for (auto dir: directions)
			{
				auto effect = user_node->pos.calc_move(dir);
				if (effect.has_changed) 
				{
					auto [new_pos, zeros] = effect.calc_pos_zeros_pair();
					auto child_node = random_node(new_pos, zeros);
					user_node->children[dir] = child_node;
					new_usernodes += child_node->children.size();
				} 
				else //illegal direction 
				{
					user_node->children[dir] = nullptr;
				}
			}
		}
		node_container->increase_level();
		return new_usernodes;
	}

}

