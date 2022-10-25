#pragma once

#include "game_play.hh"
#include <boost/container/static_vector.hpp>
#include <memory>
#include <types.hh>
#include <array>
#include <cstdint>
#include <utility>
#include <vector>


#define BIN_COUNT 32
// todo maybe not as macro? but runtime variable?

namespace c20::search {

	using namespace commons;

	/**
	 * Custom discrete distribution representation.
	 */
	struct NodeDistribution
	{
		Probability known_ending; //todo expand this
		Probability bins[BIN_COUNT];

		//Value expected_value();
		//Value standard_dev();
		//todo other marks maybe..
		// also maybe don't use any of these, but create CNN for this?

		static int bin_num_from_val(Value);
		static NodeDistribution const_dist(Value);
		static const NodeDistribution const_over_dist;
	};


	/**
	 * Wrapper around Distribution, containing numeric value.
	 * PLUS:
	 * Info on how good the evaluation is, i.e. how deep 
	 * did the search go down, how precise it got calculated
	 * etc.
	 */
	struct Evaluation
	{
		NodeDistribution dist;
		Value value;

		//int level_down;
		void add_constituent(Probability, NodeDistribution&);
	};


	/**
	 * Small wrapper around Position.
	 */
	struct Node
	{
		Position pos;
		/** Is game over at this node. */
		bool is_over = false;
        /** Is this a final node in calculating NodeDistribution? */
        bool is_final = false;

		Node();
		Node(Position);
		virtual ~Node() = default;
	};

	struct RandomNode;

	/**
	 * Node where User has to make a move.
	 */
	struct UserNode : public Node 
	{
		/** Distribution from CNN and calculated upward recursively. */
		NodeDistribution dist;
		MoveDirection best_dir;

		std::array<RandomNode*, NUM_DIRECTIONS> children;

		UserNode();
		UserNode(Position);
		virtual ~UserNode() = default;
	};

	/**
	 * Node where Random has to make a move, ie. a number 
	 * will be popped.
	 */
	struct RandomNode : public Node 
	{
		Evaluation eval;
	
		ZeroIndices zeros;
		/** Distribution of children nodes. */
		std::vector<std::pair<Probability, UserNode*>> children;

		RandomNode();
		RandomNode(Position);
		virtual ~RandomNode() = default;
	};


#define NUM_USERNODES    1<<20
#define NUM_RANDOMNODES  1<<18
	

	class NodeContainer
	{
		private:
			UserNode* usernode_buf;
			RandomNode* randomnode_buf;
			UserNode *root;
			int current_level;
			int usernode_idx, randomnode_idx;
			std::array<int, 128> usernode_levels;
			std::array<int, 128> randomnode_levels;

		public:
			NodeContainer();
			UserNode* push_usernode(Position&);
			RandomNode* push_randomnode(Position&);
			void reset(Position&);
			void increase_level();
			int last_level_length();
			std::vector<UserNode*> get_final_nodes();
			inline UserNode* root_node() {return root;};
			inline int usernode_count() {return usernode_idx;};
			std::pair<UserNode*, UserNode*> last_level_usernodes();

	};

	/** 
	 * Creates a subgraph of the game starting from the current
	 * position, passing it to the GraphEvaluator.
	 */
	class GraphSearcher 
	{
		private:
			NumberPopper popper;
			NodeContainer *node_container;

			RandomNode* random_node(Position pos, ZeroIndices& zeros);
		public:
			GraphSearcher(NumberPopper&, NodeContainer*); //gets a copy
			int search_level();
	};


	class NodeEvaluator
	{
		public:
			virtual Value evaluate(Position&) = 0;
			virtual void batch_evaluate(std::vector<UserNode*>&) = 0;
	};

	/**
	 * Evaluates graph nodes recursively, using the data 
	 * from GrapSearcher and using NeuralEvaluator at final nodes.
	 */
	class GraphEvaluator
	{
		private:
			NodeDistribution eval_usernode_recursive(UserNode*);
			void eval_randomnode_recursive(RandomNode*);
			/** Evaluate distribution, for coparison, and set member. */
			Value eval_and_set(Evaluation&);
		public:
			MoveDirection pick_one(NodeContainer*);
			/** Evaluate GameTree nodes recursively. */
			void evaluate(NodeContainer*);
	};

	class SearchManager : public core::MoveSelector {
		private:
			NodeContainer *node_container;
			GraphSearcher *graph_searcher;
			GraphEvaluator *graph_evaluator;
			NodeEvaluator *node_eval;
		public:
			SearchManager(NodeEvaluator*, NumberPopper);
			virtual UserMove make_move();
			void init(); 
			void set_position(Position);
			void start_search();
			void stop_search();
	};





}



