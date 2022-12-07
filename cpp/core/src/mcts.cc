#include "types.hh"
#include <algorithm>
#include <boost/random/mersenne_twister.hpp>
#include <boost/smart_ptr/detail/spinlock_gcc_atomic.hpp>
#include <boost/thread.hpp>
#include <boost/thread/lock_guard.hpp>
#include <cmath>
#include <cstdint>
#include <ctime>
#include <iterator>
#include <mcts.hh>
#include <spdlog/spdlog.h>
#include <vector>
#include <oneapi/tbb/parallel_for.h>

using boost::lock_guard;
using boost::container::static_vector;
using namespace oneapi::tbb;

namespace c20::mcts {

	Node::Node(const Position& pos, bool random_node) :
		pos(pos), random_node(random_node) {}
	UserNode::UserNode(const Position& pos) : Node(pos, false) {}
	RandomNode::RandomNode(const Position& pos) : 
		Node(pos, true), uct_value(10000) {}

	Node& Node::operator=(Node&& other)
	{
		pos = other.pos;
		random_node = other.random_node; 
		is_over = other.is_over;
		visit_count = other.visit_count;
		move_sum = other.move_sum;
		leef_node = other.leef_node;
		eval = other.eval;
		return *this;
	}

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
		lock_guard lock(usernode_lock);
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
		lock_guard lock(randomnode_lock);
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
		for (int i = 0; i < 8; i++)
			contexts.push_back({
				.number_popper{*number_popper}, 
				.gen{mt19937(static_cast<std::uint32_t>(std::time(0)))}
			});
	}


	Path MCTS::select(ExecutionContext& exec_context)
	{
		Path path;
		UserNode* user_node = node_container->root_node();
		user_node->lock.lock();
		RandomNode* random_node;

		while (1)
		{
			path.push_back(user_node);
			if (user_node->leef_node)
				break;
			random_node = choose_child(user_node);
			user_node->lock.unlock();
			random_node->lock.lock();
			path.push_back(random_node);
			if (random_node->leef_node)
				break;
			user_node = choose_child(exec_context, random_node);
			random_node->lock.unlock();
			user_node->lock.lock();
		}
		return path;
	}


	RandomNode* MCTS::choose_child(UserNode* node)
	{
		int parent_visit_count = node->visit_count;
		auto const_C_copy = const_C; //to eliminiate race condition
		static_vector<Value, 4> uct_values;
		for (auto child: node->children) 
		{
			if (child == nullptr) {
				uct_values.push_back(-1);
				continue;
			}
			//TODO there is a race condition here, maybe not problematic
			Value uct_val = 
				child->eval + 
				const_C_copy * std::sqrt(
					std::log(parent_visit_count +1) / (child->visit_count +1))
				;
			uct_values.push_back(uct_val);
		}
		unsigned max_idx = std::distance(uct_values.begin(),
				std::max_element(uct_values.begin(), uct_values.end()));
		auto chosen_child = node->children[max_idx];
		assert(chosen_child != nullptr);
		return chosen_child;
	}

	UserNode*  MCTS::choose_child(
			ExecutionContext& exec_context, RandomNode* random_node)
	{
		int idx = random_node->children_dist(exec_context.gen);
		return random_node->children[idx];
	}

	Node* MCTS::expand(ExecutionContext& exec_context, Path& path) 
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
			auto dist = exec_context.number_popper.dist_from(pos, zeros);
			double dist_probs[NUM_SQUARES *2];
			for (unsigned i = 0; i < dist.size(); i++) {
				auto& [p, child_pos] = dist[i];
				dist_probs[i] = p;
				auto child_node = node_container->push_usernode(child_pos);
				random_node->children.push_back(child_node);
			}

			random_node->children_dist = disc_dist(dist_probs, dist_probs +dist.size());
			
			//return random
			int idx = random_node->children_dist(exec_context.gen);
			auto child = random_node->children[idx];
			path.push_back(child);
			random_node->lock.unlock();
			return child;
		}
		else 
		{
			auto user_node = static_cast<UserNode*>(last_node);
			if (user_node->is_over) {
				user_node->lock.unlock();
				return user_node;
			}

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
			auto best_child = *std::max_element(
					user_node->children.begin(), user_node->children.end(),
					[] (auto node1, auto node2) {
						if (node1 == nullptr) return true;
						if (node2 == nullptr) return false;
						return node1->pos_score < node2->pos_score;
					});
			path.push_back(best_child);
			user_node->lock.unlock();
			return best_child;
		}
	}

	uint32_t MCTS::rollout(ExecutionContext& exec_context, Node* start_node)
	{
		Position pos = start_node->pos;
		int move_count = 0;
		if (start_node->random_node) {
			exec_context.number_popper.place_one(pos);
			move_count = 1;
		}

		for (int i = 0; i < 1; i++) {
			Position pos_copy = pos;
			move_count += 2 * rollout_pos(exec_context, pos_copy);
		}
		return move_count;		
	}

	uint32_t MCTS::rollout_pos(ExecutionContext& exec_context, Position& pos)
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
				int dir_num = dist(exec_context.gen);
				MoveDirection random_dir = MoveDirection(dir_num);
				auto result = pos.calc_move(random_dir);
				if (result.has_changed)
				{
					std::tie(pos, zeros) = result.calc_pos_zeros_pair(); 
					break; //inner
				}
			}

			exec_context.number_popper.place_one(pos, zeros);
			move_count++;
		}
		return move_count;
	}


	void MCTS::back_propagate(Path& path, uint32_t move_count)
	{
		double highest_score = -1.;
		double loc_max_eval = -1;
		
		for (int i = path.size() -1; i >= 0; i--)
		{
			auto node = path[i];
			lock_guard<spinlock> lock(node->lock);
			node->visit_count++;
			if (node->random_node) {
				auto rnode = static_cast<RandomNode*>(node);
				highest_score = std::max(highest_score, rnode->pos_score);
				double mult = 1 + highest_score /
					std::pow(rnode->visit_count, params.decline_pow);
				node->move_sum += move_count * mult;
				node->eval = (double)(node->move_sum) / node->visit_count;
				loc_max_eval = std::max(loc_max_eval, node->eval);
			}
			move_count++;
		}

		lock_guard<spinlock> lock(const_C_lock);
		max_eval = std::max(max_eval, loc_max_eval);
		const_C = params.const_C_mult * max_eval;
	}

	void MCTS::mcts_loop(ExecutionContext& context, int repeats)
	{
		for (int i = 0; i < repeats; i++) {
			auto path = select(context);
			auto leef_node = expand(context, path);
			if (leef_node->is_over) {
				back_propagate(path, 0);
				continue;
			}
			auto move_count = rollout(context, leef_node);
			back_propagate(path, move_count);
		}
	}


	UserMove MCTS::make_move()
	{
		node_container->reset(pos_);
		const_C = 10000;
		max_eval = -1;

		int big_nums = pos_.count_above(4) + pos_.count_above(6);
		int huge_nums = pos_.count_above(7);
		int time = pos_.power_sum() / 1000 + big_nums + 2 * huge_nums + 1;
		time += 2* std::pow(std::max(0, 6 - pos_.num_zeros()), 2);
		auto main_path = node_eval->find_main_path(pos_);
		time += std::max(0, 4 - main_path.len) * 5;



		int iterations = 4 * time;

		int checkpoints = 3;
		int parralellism = 2;
		int it_per_checkp = iterations / checkpoints;
		auto thread_it = it_per_checkp / parralellism + 1;
		for (int cp = 0; cp < 5 * checkpoints; cp ++) {
			/* parallel_for(blocked_range<size_t>(0, parralellism, 1), */
			/* 		[&] (const auto& r) { */
			auto r = blocked_range<size_t>(0, parralellism);
						for (auto idx=r.begin(); idx != r.end(); idx++)
							mcts_loop(contexts[(int)idx], thread_it);
					/* }); */
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


