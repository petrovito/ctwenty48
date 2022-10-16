#include <types.hh>
#include <array>
#include <cnn.h>
#include <cstdint>
#include <utility>
#include <vector>


#define BIN_COUNT 16
// todo maybe not as macro? but runtime variable?

namespace c20::search {

	using namespace commons;

	typedef double Value;


	/**
	 * Custom discrete distribution representation.
	 */
	struct NodeDistribution
	{
		Probability known_ending; //todo expand this
		Probability bins[BIN_COUNT];

		Value expected_value();
		Value standard_dev();
		//todo other marks maybe..
		// also maybe don't use any of these, but create CNN for this?

		static NodeDistribution const_dist(Value);
	};

	/**
	 * Small wrapper around Position.
	 */
	struct Node
	{
		Position pos;
		/** Distribution from CNN and calculated upward recursively. */
		NodeDistribution distribution;

		Node(Position);
	};

	struct RandomNode;

	/**
	 * Node where User has to make a move.
	 */
	struct UserNode : public Node 
	{
		std::array<RandomNode*, NUM_DIRECTIONS> children;
		/** Is game over at this node. */
		bool is_over = false;
		/** Is this a final node in calculating NodeDistribution? */
		bool is_final = false;

		UserNode(Position);
	};

	/**
	 * Node where Random has to make a move, ie. a number 
	 * will be popped.
	 */
	struct RandomNode : public Node 
	{
		ZeroIndices zeros;
		/** Distribution of children nodes. */
		std::vector<std::pair<Probability, UserNode*>> children;

		RandomNode(Position);
	};


	/** Class representing logic which Move to choose at UserNodes. */
	class UserMoveSelector
	{
		public:
			virtual MoveDirection choose(UserNode);
	};

	/** Selects the best expected value. */
	class ExpectedValueSelector : public UserMoveSelector
	{ };

	/** Selects random user move. */
	class RandomSelector : public UserMoveSelector 
	{ };

	/** 
	 * Creates a subgraph of the game starting from the current
	 * position, passing it to the GraphEvaluator.
	 */
	class GraphSearcher 
	{
		private:
			NumberPopper popper;
			RandomNode* random_node_recursive(Position, ZeroIndices&, int);
			UserNode* user_node_recursive(Position, int);
		public:
			/** 
			 * Find a subgraph from a given position and a given depth.
			 * Returns a UserNode representing the supplied Position,
			 * with the pointers to the children nodes.
			 */
			UserNode subgraph_of_depth(Position&, int);
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
		Value eval;

		int level_down;
	};

	/**
	 * Evaluates graph nodes recursively, using the data 
	 * from GrapSearcher and using NeuralEvaluator at final nodes.
	 */
	class GraphEvaluator
	{
		public:
			MoveDirection pick_one();
			Evaluation evaluate();
	};

	class SearchManager {
		private:
			GraphSearcher *graph_searcher;
			GraphEvaluator *graph_evaluator;
			cnn::NeuralEvaluator *cnn;
		public:
			void init(); 
			void set_position(Position);
			void start_search();
			void stop_search();
	};





}



