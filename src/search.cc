#include <algorithm>
#include <limits>
#include <types.hh>
#include <search.h>
#include <utility>


namespace c20::search {



	UserNode::UserNode(Position pos) : Node(pos) {}
	RandomNode::RandomNode(Position pos) : Node(pos) {}
	Node::Node(Position pos) : pos(pos) {}



	UserNode GraphSearcher::subgraph_of_depth(Position& pos, int depth)
	{
		UserNode start_node(pos);
		for (auto dir: directions)
		{
			auto effect = pos.calc_move(dir);
			if (effect.has_changed) 
			{
				auto [new_pos, zeros] = 
					effect.calc_pos_zeros_pair();
				start_node.children[dir] = 
					random_node_recursive(new_pos, zeros, depth);
			} 
			else //illegal direction 
			{
				start_node.children[dir] = nullptr;
			}
		}
		return start_node;
	}



	RandomNode* GraphSearcher::random_node_recursive(
			Position pos, ZeroIndices& zeros, int depth)
	{
		auto random_node = new RandomNode(pos);
		random_node->children.push_back(std::make_pair(0, nullptr));
		for (auto [prob, child_pos]: popper.dist_from(pos, zeros))
		{

		}
		return random_node;
	}



}

