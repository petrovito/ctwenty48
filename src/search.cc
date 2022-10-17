#include <limits>
#include <utility>


#include <types.hh>
#include <search.hh>


namespace c20::search {



	UserNode::UserNode(Position pos) : Node(pos) {}
	RandomNode::RandomNode(Position pos) : Node(pos) {}
	Node::Node(Position pos) : pos(pos) {}


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



}

