#include "cnn.hh"
#include "types.hh"
#include <cstdlib>
#include <gtest/gtest.h>

namespace c20::cnn {
	using namespace commons;
	using namespace std;

#define PRINT 0

#if PRINT
#define EVAL(POS) \
	{ \
		Position pos = Position::from_str(POS); \
		cout << POS << ": " << evaluator->evaluate(pos) << endl; \
	} 
#else 
#define EVAL(POS) \
	{ \
		Position pos = Position::from_str(POS); \
		evaluator->evaluate(pos); \
	} 
#endif

	TEST(CnnTest, EvaluatePos)
	{
		auto evaluator = NeuralEvaluator::load_from("neural-net/models/v1");
		
		EVAL("1234|5678|0000|0001");
		EVAL("1234|5678|9371|0001");
		EVAL("1234|0000|0000|0001");
		EVAL("1342|5678|1975|2371");
	}

}

