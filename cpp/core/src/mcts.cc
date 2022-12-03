#include "types.hh"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <ctime>
#include <iterator>
#include <mcts.hh>
#include <spdlog/spdlog.h>
#include <vector>

namespace c20::mcts {

	Node::Node(const Position& pos, bool random_node) :
		pos(pos), random_node(random_node) {}
	UserNode::UserNode(const Position& pos) : Node(pos, false) {}
	RandomNode::RandomNode(const Position& pos) : 
		Node(pos, true), uct_value(10000) {}

//NodeContainer class

	NodeContainer::NodeContainer() : 
		usernode_buf(new UserNode[MCTS_NUM_USERNODES]),
		randomnode_buf(new RandomNode[MCTS_NUM_RANDOMNODES]),
		root(usernode_buf)
	{}

	NodeContainer::~NodeContainer()
	{
		delete[] usernode_buf;
		delete[] randomnode_buf; 
	}

	UserNode* NodeContainer::push_usernode(Position& pos)
	{
		if (usernode_idx == MCTS_NUM_USERNODES)
			throw std::runtime_error("Index out of bound.");
		if (usernode_map.count(pos))
		{
			return usernode_map[pos];
		}
		usernode_buf[usernode_idx] = UserNode(pos);
		auto ptr = &usernode_buf[usernode_idx];
		ptr->is_over = pos.is_over();

		usernode_idx++;
		usernode_map[pos] = ptr;
		return ptr;
	}


	RandomNode* NodeContainer::push_randomnode(Position& pos, Value score)
	{
		if (randomnode_idx == MCTS_NUM_RANDOMNODES)
			throw std::runtime_error("Index out of bound.");
		if (randomnode_map.count(pos))
		{
			return randomnode_map[pos];
		}
		randomnode_buf[randomnode_idx] = RandomNode(pos);
		auto ptr = &randomnode_buf[randomnode_idx];
		ptr->pos_score = score;

		randomnode_idx++;
		randomnode_map[pos] = ptr;
		return ptr;
	}


	void NodeContainer::reset(const Position& pos)
	{
		usernode_buf[0] = UserNode(pos);
		usernode_idx = 1;
		randomnode_idx = 0;
		usernode_map.clear();
		randomnode_map.clear();
	}

//MCTS

	MCTS::MCTS() : 
		gen(static_cast<std::uint32_t>(std::time(0))),
		uniform(0,3)
	{ }


	void MCTS::init()
	{
		uint8_t mask = 0b1111;
		for (uint8_t subset = 0; subset < mask; subset++) {
			std::vector<double> weights;
			for (MoveDirection dir: directions) {
				if (subset & (1 << dir)) {
					weights.push_back(params.rollout_corner_weight);
				} else {
					weights.push_back(1.);
				}
			}
			dist_cache.push_back({weights.begin(), weights.end()});
		}
	}


	Path MCTS::select()
	{
		Path path;
		UserNode* user_node = node_container->root_node();
		RandomNode* random_node;

		while (1)
		{
			path.push_back(user_node);
			if (user_node->leef_node)
				break;
			random_node = choose_child(user_node);
			path.push_back(random_node);
			if (random_node->leef_node)
				break;
			user_node = choose_child(random_node);
		}
		return path;
	}


	RandomNode* MCTS::choose_child(UserNode* node)
	{
		int parent_visit_count = node->visit_count;
		for (auto child: node->children) 
		{
			if (child == nullptr) continue;
			child->uct_value = 
				child->eval + 
				const_C * std::sqrt(
					std::log(parent_visit_count) / (child->visit_count))
				;
		}
		return *std::max_element(node->children.begin(), node->children.end(),
				[] (auto node1, auto node2) { 
					if (node1 == nullptr) return true;
					if (node2 == nullptr) return false;
					return node1->uct_value < node2->uct_value;
				});
	}

	UserNode*  MCTS::choose_child(RandomNode* random_node)
	{
		int idx = random_node->children_dist(gen);
		return random_node->children[idx];
	}

	Node* MCTS::expand(Path& path) 
	{
		auto& last_node = path.back();
		if (last_node->random_node)
		{
			auto random_node = static_cast<RandomNode*>(last_node);
			random_node->leef_node = false;

			auto& pos = random_node->pos;
			ZeroIndices zeros;
			for (int i = 0; i < NUM_SQUARES; i++)
			{
				if (pos[i] == 0) zeros.push_back(i);
			}
			auto dist = number_popper->dist_from(pos, zeros);
			double dist_probs[dist.size()];
			for (int i = 0; i < dist.size(); i++) {
				auto& [p, child_pos] = dist[i];
				dist_probs[i] = p;
				auto child_node = node_container->push_usernode(child_pos);
				random_node->children.push_back(child_node);
			}

			random_node->children_dist = disc_dist(dist_probs, dist_probs +dist.size());
			
			//return random
			int idx = random_node->children_dist(gen);
			return random_node->children[idx];
		}
		else 
		{
			auto user_node = static_cast<UserNode*>(last_node);
			if (user_node->is_over) return user_node;

			user_node->leef_node = false;

			for (auto dir: directions)
			{
				auto effect = user_node->pos.calc_move(dir);
				if (effect.has_changed) 
				{
					auto [new_pos, zeros] = effect.calc_pos_zeros_pair();
					Value pos_score = node_eval->evaluate(new_pos);
					auto child_node = node_container->push_randomnode(new_pos, pos_score);
					user_node->children[dir] = child_node;
				} 
				else //illegal direction 
				{
					user_node->children[dir] = nullptr;
				}
			}

			//TODO make it pseudorandom
			auto max = *std::max_element(user_node->children.begin(), user_node->children.end(),
					[] (auto node1, auto node2) {
						if (node1 == nullptr) return true;
						if (node2 == nullptr) return false;
						return node1->pos_score < node2->pos_score;
					});
			return max;
			/* while (1) */
			/* { */
			/* 	int dir_num = uniform(gen); */
			/* 	MoveDirection random_dir = MoveDirection(dir_num); */
			/* 	if (user_node->children[random_dir]) */
			/* 		return user_node->children[random_dir]; */
			/* } */
		}
	}

	uint32_t MCTS::rollout(Node* start_node)
	{
		Position pos = start_node->pos;
		int move_count = 0;
		if (start_node->random_node) {
			number_popper->place_one(pos);
			move_count = 1;
		}

		for (int i = 0; i < 1; i++) {
			Position pos_copy = pos;
			move_count += 2 * rollout_pos(pos_copy, *number_popper, gen);
		}
		return move_count;		
	}

	uint32_t MCTS::rollout_pos(Position& pos,
			NumberPopper& popper, boost::mt19937& gen)
	{
		int move_count = 0;
		ZeroIndices zeros;
		while (1) 
		{
			if (pos.is_over()) break;
			//make random move
			uint8_t weight_idx = 0;
			auto highest_idx = pos.highest_idx();
			if (highest_idx == 0 || highest_idx == 3)
				weight_idx |= 1 << UP;
			if (highest_idx == 15 || highest_idx == 3)
				weight_idx |= 1 << RIGHT;
			if (highest_idx == 0 || highest_idx == 12)
				weight_idx |= 1 << LEFT;
			if (highest_idx == 12 || highest_idx == 15)
				weight_idx |= 1 << DOWN;
			disc_dist& dist = dist_cache[weight_idx];
			while (1)
			{
				int dir_num = dist(gen);
				MoveDirection random_dir = MoveDirection(dir_num);
				auto result = pos.calc_move(random_dir);
				if (result.has_changed)
				{
					std::tie(pos, zeros) = result.calc_pos_zeros_pair(); 
					break; //inner
				}
			}

			popper.place_one(pos, zeros);
			move_count++;
		}
		return move_count;
	}


	void MCTS::back_propagate(Path& path, uint32_t move_count)
	{
		double highest_score = -1.;
		
		for (int i = path.size() -1; i >= 0; i--)
		{
			auto node = path[i];
			node->visit_count++;
			if (node->random_node) {
				auto rnode = static_cast<RandomNode*>(node);
				highest_score = std::max(highest_score, rnode->pos_score);
				double mult = 1 + highest_score /
					std::pow(rnode->visit_count, params.decline_pow);
				node->move_sum += move_count * mult;
				node->eval = (double)(node->move_sum) / node->visit_count;
				max_eval = std::max(max_eval, node->eval);
			}
			move_count++;
		}

		const_C = params.const_C_mult * max_eval;
	}


	UserMove MCTS::make_move()
	{
		node_container->reset(pos_);
		const_C = 10000;
		max_eval = -1;

		int big_nums = pos_.count_above(4) + pos_.count_above(6);
		int time = pos_.power_sum() / 1000 + big_nums + 1;
		if (pos_.num_zeros() < 3) time *= 2;
		int iterations = 45 * time;

		int checkpoints = 5;
		int it_per_checkp = iterations / checkpoints;
		for (int cp = 0; cp < 5 * checkpoints; cp ++) {
			for (int i = 0; i < it_per_checkp; i++) {
				auto path = select();
				auto leef_node = expand(path);
				if (leef_node->is_over) {
					back_propagate(path, 0);
					continue;
				}
				auto move_count = rollout(leef_node);
				back_propagate(path, move_count);
			}
			std::vector<int> visit_counts;
			for (auto child: node_container->root_node()->children)
				if (child)
					visit_counts.push_back(child->visit_count);
				else visit_counts.push_back(-1);
			std::sort(visit_counts.begin(), visit_counts.end());
			if (visit_counts[3] > (double)(visit_counts[2]) *(1+2./(1+cp)))
				break;
			/* else { */
			/* 	if (cp == checkpoints -1) */
			/* 		spdlog::error("NOT EARLY RETURN!"); */
			/* } */
		}
		/* spdlog::info("{},{}, {}", node_container->root_node()->eval, node_container->root_node()->visit_count, node_container->usernode_count()); */
		/* std::vector<int> visit_counts; */ 
		/* for (auto child: node_container->root_node()->children) */
		/* 	if (child) */
		/* 		visit_counts.push_back(child->visit_count); */
		/* 	else visit_counts.push_back(-1); */
		/* spdlog::info("[{},{},{},{}", visit_counts[0], visit_counts[1], visit_counts[2], visit_counts[3]); */		
		/* std::vector<Value> values; */ 
		/* for (auto child: node_container->root_node()->children) */
		/* 	if (child) */
		/* 		values.push_back(child->eval); */
		/* 	else values.push_back(-1); */
		/* spdlog::info("[{},{},{},{}", values[0], values[1], values[2], values[3]); */



		auto moves = node_container->root_node()->children;
		auto best_child = std::max_element(moves.begin(), moves.end(),
				[] (auto x, auto y) {
					if (x == nullptr) return true;
					if (y == nullptr) return false;
					return x->visit_count < y->visit_count;
				});
		auto best_dir = MoveDirection(std::distance(moves.begin(), best_child));
		return UserMove{.direction=best_dir};
	}

}


