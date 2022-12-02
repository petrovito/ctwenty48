#pragma once

#include "types.hh"
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <cstdint>
#include <memory>
#include <selectors.hh>
#include <search.hh>
#include <vector>

namespace c20::mcts {
	
	using namespace commons;

//TODO lot of duplicatad code here

	struct Node
	{
		Position pos;
		bool random_node;
		/** Is game over at this node. */
		bool is_over = false;

		uint32_t visit_count = 0;
		/** Sum of all moves made below this node. */
		double move_sum = 0;
		bool leef_node = true;
		Value eval = 0;

		Node() = default;
		Node(const Position&, bool);
		virtual ~Node() = default;
	};

	struct RandomNode;

	/**
	 * Node where User has to make a move.
	 */
	struct UserNode : public Node
	{
		std::array<RandomNode*, NUM_DIRECTIONS> children;

		UserNode() = default;
		UserNode(const Position&);
	};

	typedef boost::random::discrete_distribution<> disc_dist;
	/**
	 * Node where Random has to make a move, ie. a number 
	 * will be popped.
	 */
	struct RandomNode : public Node
	{
		double uct_value;
		double pos_score;

		/** Distribution of children nodes. */
		std::vector<UserNode*> children;
		disc_dist children_dist;

		RandomNode() = default;
		RandomNode(const Position&);
	};


#define MCTS_NUM_USERNODES    1<<22
#define MCTS_NUM_RANDOMNODES  1<<20
	

	class NodeContainer
	{
		private:
			UserNode* usernode_buf;
			RandomNode* randomnode_buf;
			UserNode *root;
			int usernode_idx, randomnode_idx;

			std::unordered_map<Position, UserNode*, PositionHasher> usernode_map;
			std::unordered_map<Position, RandomNode*, PositionHasher> randomnode_map;

		public:
			NodeContainer();
			~NodeContainer();
			UserNode* push_usernode(Position&);
			RandomNode* push_randomnode(Position&, Value score);
			void reset(const Position&);
			inline UserNode* root_node() {return root;};
			inline int usernode_count() {return usernode_idx;};
	};


	typedef std::vector<Node*> Path;

	struct MctsHyperParams
	{
		//mcts ops
		double const_C_mult = std::sqrt(2);
		double decline_pow = .75;
		double rollout_corner_weight = 2.;
		//scoring
		std::vector<double> main_path_mults{1,2,4,8,26,40};
		double breaker_mult = 2.7;
		double breaker_pow = 2.;
	};


	class MCTS : public selectors::MoveSelector
	{
		private:
			NodeContainer* node_container;
			NumberPopper* number_popper;
			search::NodeEvaluator* node_eval;
			boost::random::mt19937 gen;
			boost::random::uniform_int_distribution<> uniform;

			RandomNode* choose_child(UserNode*);
			UserNode* choose_child(RandomNode*);
			uint32_t rollout_pos(Position&, 
					NumberPopper&, boost::random::mt19937&);

			Path select();
			Node*  expand(Path&);
			uint32_t rollout(Node*);
			void back_propagate(Path&, uint32_t);

			double const_C;
			double max_eval;

			MctsHyperParams params;
			

			template<typename UiEnv> friend class deps::Environment;
		public:
			MCTS();
			UserMove make_move() override;
	};


}

