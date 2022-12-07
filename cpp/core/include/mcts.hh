#pragma once

#include "pos_score.hh"
#include "types.hh"
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/smart_ptr/detail/spinlock_gcc_atomic.hpp>
#include <cstdint>
#include <memory>
#include <mutex>
#include <selectors.hh>
#include <search.hh>
#include <vector>

using boost::detail::spinlock;
/* typedef std::mutex spinlock; */
/* struct spinlock { */
/* 	bool locked = false; */
/* 	inline void lock() { */
/* 		if (locked) { */
/* 			locked = true; */
/* 		} else { */
/* 			locked = true; */
/* 		} */
/* 	} */ 
/* 	inline void unlock() { */
/* 		if (!locked) { */
/* 			locked = false; */
/* 		} */
/* 		locked = false; */
/* 	} */

/* }; */

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

		spinlock lock = {};

		Node() = default;
		Node(const Position&, bool);
		Node& operator=(Node&&);
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

			spinlock usernode_lock = {};
			spinlock randomnode_lock = {};

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

	using boost::random::mt19937;

	struct ExecutionContext
	{
		NumberPopper number_popper;
		mt19937 gen;
	};


	class MCTS : public selectors::MoveSelector
	{
		private:
			NodeContainer* node_container;
			NumberPopper* number_popper;
			search::StaticPositionEval* node_eval;
			boost::random::uniform_int_distribution<> uniform;
			std::vector<disc_dist> dist_cache;
			std::vector<ExecutionContext> contexts;
			/* ExecutionContext context; */

			spinlock const_C_lock = {};
			double const_C;
			double max_eval;

			search::MctsHyperParams params;


			RandomNode* choose_child(UserNode*);
			UserNode* choose_child(ExecutionContext&, RandomNode*);
			uint32_t rollout_pos(ExecutionContext&, Position&);

			Path select(ExecutionContext&);
			Node*  expand(ExecutionContext&, Path&);
			uint32_t rollout(ExecutionContext&, Node*);
			void back_propagate(Path&, uint32_t);

			void mcts_loop(ExecutionContext&, int);


			template<typename UiEnv> friend class deps::Environment;
		public:
			MCTS();
			void init(); //to call after wiring beans
			UserMove make_move() override;
	};


}

