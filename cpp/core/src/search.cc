#include "cnn.hh"
#include <array>
#include <iterator>
#include <limits>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <utility>


#include <types.hh>
#include <search.hh>
#include <vector>


namespace c20::search {



	UserNode::UserNode(const Position& pos) : Node(pos), children({}), dist({}) {}
	RandomNode::RandomNode(const Position& pos) : Node(pos), eval({}) {}
	Node::Node(Position pos) : pos(pos), is_final(false), is_over(false) {}

	UserNode::UserNode() : Node(), children({}), dist({}) {}
	RandomNode::RandomNode() : Node(), children({}), eval({}) {}
	Node::Node() : pos({}), is_final(false), is_over(false) {}


	GraphSearcher::GraphSearcher(NumberPopper* _popper, NodeContainer* nc) :
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


#define USE_MULT       1

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

#if USE_MULT
		double multiplier = 1;
		auto highests = random_node->pos.highest(3);
		auto first = highests[0];
		if (
				first.idx == 0 || first.idx == 3 || 
				first.idx == 12 || first.idx == 15
		) {
			multiplier *= 2.5;
			auto second = highests[1];
			auto idx_diff = first.idx - second.idx;
			auto idx_diff_abs = std::abs(idx_diff);
			if (idx_diff_abs == 1 || idx_diff_abs == TABLE_SIZE) {
				multiplier *= 1.5;
				auto third = highests[2];
				auto idx_diff_2 = second.idx - third.idx;
				if (idx_diff == idx_diff_2)
					multiplier *= 1.2;
			}
			
			random_node->eval.value *= multiplier;
			/* for (int i = BIN_COUNT -1; i >= 0; i--) */
			/* { */
			/* 	int target = std::min(BIN_COUNT -1, (int)(i*multiplier/2)); */
			/* 	random_node->eval.dist.bins[target] += */ 
			/* 		random_node->eval.dist.bins[i]; */
			/* 	random_node->eval.dist.bins[i] = 0.; */
			/* } */
		}
#endif
	}

#define USE_PERCENTILE 0
#define USE_EV         1

	Value GraphEvaluator::eval_and_set(Evaluation& eval)
	{
#if USE_EV
		eval.value = -1 * eval.dist.known_ending;
		for (int i = 0; i < BIN_COUNT; i++)
		{
			auto bin_prob = eval.dist.bins[i];
			eval.value += bin_prob * i;
		}
#elif USE_PERCENTILE
		float upper_bound = 0.7;
		float cum_prob = eval.dist.known_ending;
		float prev_cum_prob = cum_prob;
		for (int i = 0; i < BIN_COUNT; i++)
		{
			cum_prob += eval.dist.bins[i];
			if (cum_prob > upper_bound) {
				float weight = upper_bound - prev_cum_prob;
				eval.value += weight *i;
				break;
			} else {
				eval.value += i * eval.dist.bins[i];
			}
			prev_cum_prob = cum_prob;
		}
#endif
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


//SearchManager class

	
	
	SearchManager::SearchManager(NodeEvaluator* _node_eval,
			NumberPopper* _popper) : 
		node_container(new NodeContainer()),
		graph_searcher(new GraphSearcher(_popper, node_container)), 
		graph_evaluator(new GraphEvaluator()),
		node_eval(_node_eval) {  }

	UserMove SearchManager::make_move()
	{
		node_container->reset(pos);

		int pow_sum = pos.power_sum();
		int medium_nums = pos.count_above(5);
		int big_nums = pos.count_above(7);
		int bigger_nums = pos.count_above(9);

		TimeSpan time = 1 + (medium_nums + bigger_nums + 2*big_nums)/3 + pow_sum /2000;

		int depth = 1;
		int max_node_count = 10000;
		
		switch (time) {
			case 0:
			case 1:
			case 2:
				depth = 1;
				break;
			case 3:
			case 4:
			case 5:
				depth = 2;
				break;
			case 6:
				depth = 3;
				break;
			case 7:
				depth = 5;
				max_node_count = 20000;
				break;
			case 8:
				depth = 5;
				max_node_count = 30000;
				break;
			default:
				depth = 5;
				max_node_count = 40000;
				break;
		}
		
		for (int i = 0; i < depth; i++) {
			if (node_container->usernode_count() > max_node_count) break;
			int new_nodes = graph_searcher->search_level();
			if (new_nodes == 0) break;
		}
		
		auto final_nodes = node_container->get_final_nodes();
		node_eval->batch_evaluate(final_nodes, time);

		graph_evaluator->evaluate(node_container);
		auto dir = graph_evaluator->pick_one(node_container);
		return UserMove{dir};
	}


	Analysis SearchManager::analyze(const Position& pos) 
	{
		std::lock_guard lock(analyze_mutex);
		Analysis anal;
		node_container->reset(pos);

		anal.position_val = node_eval->evaluate(pos) * BIN_COUNT;

		for (int i = 0; i < 3; i++) {
			if (node_container->usernode_count() > 9000) break;
			int new_nodes = graph_searcher->search_level();
			if (new_nodes == 0) break;
			auto final_nodes = node_container->get_final_nodes();
			node_eval->batch_evaluate(final_nodes, 10);
			graph_evaluator->evaluate(node_container);
			anal.deep_values.push_back(node_container->children_evals());
		}
		return anal;
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
		if (usernode_map.count(pos))
		{
			return usernode_map[pos];
		}
		usernode_buf[usernode_idx] = UserNode(pos);
		auto ptr = &usernode_buf[usernode_idx];

		usernode_idx++;
		usernode_map[pos] = ptr;
		return ptr;
	}


	RandomNode* NodeContainer::push_randomnode(Position& pos)
	{
		if (randomnode_idx == NUM_RANDOMNODES)
			throw std::runtime_error("Index out of bound.");
		if (randomnode_map.count(pos))
		{
			return randomnode_map[pos];
		}
		randomnode_buf[randomnode_idx] = RandomNode(pos);
		auto ptr = &randomnode_buf[randomnode_idx];

		randomnode_idx++;
		randomnode_map[pos] = ptr;
		return ptr;
	}


	void NodeContainer::reset(const Position& pos)
	{
		current_level = 0;
		usernode_levels[0] = 1;
		randomnode_levels[0] = 0;
		usernode_buf[0] = UserNode(pos);
		usernode_idx = 1;
		randomnode_idx = 0;
		usernode_map.clear();
		randomnode_map.clear();
	}

	void NodeContainer::increase_level()
	{
		current_level++;
		usernode_levels[current_level] = usernode_idx;
		randomnode_levels[current_level] = randomnode_idx;
		usernode_map.clear();
		randomnode_map.clear();
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


	std::array<Value, NUM_DIRECTIONS> NodeContainer::children_evals()
	{
		std::array<Value, NUM_DIRECTIONS> evals;
		for (int i = 0; i < NUM_DIRECTIONS; i++)
		{
			if (root->children[i])
				evals[i] = root->children[i]->eval.value;
			else evals[i] = 0;
		}
		/* spdlog::info("{}:{}:{}:{}", evals[0], evals[1],evals[2], evals[3]); */
		return evals;
	}

//GraphSearcher class

	RandomNode* GraphSearcher::random_node(Position pos, ZeroIndices& zeros)
	{
		auto random_node = node_container->push_randomnode(pos);

		//if random node was already contained the children are populated already
		if (random_node->children.size()) return random_node;
		
		for (auto [prob, child_pos]: popper->dist_from(pos, zeros))
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

