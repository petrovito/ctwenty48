#include "types.hh"
#include <array>
#include <cnn.h>
#include <cstdint>


#define BIN_COUNT 16
// todo maybe not as macro? but runtime variable?

namespace c20::search {

	using namespace commons;

	typedef double Probability;
	typedef double Value;

	struct Node
	{
		
	};

	struct UserNode : public Node 
	{

	};

	struct RandomNode : public Node 
	{

	};

	struct NodeDistibution
	{
		Probability known_ending; //todo expand this
		Probability bins[BIN_COUNT];

		Value expected_value();
		Value standard_dev();
		//todo other marks maybe..
		// also maybe don't use any of these, but create CNN for this?
	};

	struct UserNodeValueSet 
	{
		uint8_t valid_move_set;
		std::array<NodeDistibution, NUM_DIRECTIONS> dists;
	};

	class UserNodeSelector
	{
		public:
			virtual MoveDirection choose(UserNodeValueSet);
	};

	struct RandomNodeResult 
	{

	};


	class GraphSearcher 
	{

	};


	class GraphEvaluator
	{

	};

	class SearchManager {
		private:
			GraphSearcher *graph_searcher;
			GraphEvaluator *graph_evaluator;
			cnn::NeuralEvaluator *cnn;
		public:
			void init(); void set_position(Position);
			void start_search();
			void stop_search();
	};





}



