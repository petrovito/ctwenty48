
#pragma once

#include "gtest/gtest_prod.h"
#include <boost/random/discrete_distribution.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/container/static_vector.hpp>

#include <boost/random/uniform_int_distribution.hpp>
#include <cstddef>
#include <cstdint>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#define MAX_NUMBERS 16
#define MAX_MOVES 65536
#define TABLE_SIZE 4
#define NUM_SQUARES TABLE_SIZE*TABLE_SIZE

using boost::random::mt19937;
using boost::random::discrete_distribution;

namespace c20::deps { 
	class NoopUiEnv;
	template<typename UiEnv=NoopUiEnv>
	class Environment;
}
	

namespace c20::commons {

	extern volatile bool shutting_down;

	typedef uint8_t Number;

	enum GeneralDirection {
		VERTICAL,
		HORIZONTAL,
	};

	enum MoveDirection 
	{
		UP,
		DOWN,
		LEFT,
		RIGHT,
		NUM_DIRECTIONS=4
	};


	const static MoveDirection directions[] = {UP, DOWN, LEFT, RIGHT};


	struct MoveResultSegment 
	{
		public:
			Number new_segment[TABLE_SIZE];
			
			bool has_changed = false;
			int non_zero = 0;

			MoveResultSegment(); 
			void push_back(Number, int);
			Number& operator[](int);
		private:
			Number current_merge_candidate = 0;
	};

	class Position;
	typedef boost::container::static_vector<int, NUM_SQUARES> ZeroIndices;

	/**
	 * Set of MoveResultSegment for all segments (along direction).
	 */
	struct MoveResultSet
	{
		MoveResultSegment segment_results[TABLE_SIZE];
		bool has_changed;
		MoveDirection dir;
		MoveResultSegment& operator[](int);

		std::tuple<Position, ZeroIndices> calc_pos_zeros_pair();
	};

	struct UserMove 
	{
		MoveDirection direction;	
	};

	enum MoveResultType 
	{
		SUCCES,
		INVALID,
		GAME_OVER,
	};

	
	class SquareIterator
	{
		private:
			const Number* _begin;
			const Number* _end;
		public:
			SquareIterator(const Number*, const Number*);
			const Number* begin();
			const Number* end();
	};

	struct NumPos
	{
		int idx;
		Number num;
	};

	class Position 
	{
		private:
			Number table[TABLE_SIZE][TABLE_SIZE];		

			/**
			 * A segment is a row/column.
			 * More precisely is a view of that, i.e. indexing might reversed
			 * if direction is DOWN or RIGHT. 
			 * See also: start_indices, deltas.
			 */
			MoveResultSegment calc_move_segment(MoveDirection, int) const;
		public:
			/** Calculates views for move along direction. */
			MoveResultSet calc_move(MoveDirection) const;

			/** Are there any more legal moves? */
			bool is_over() const;

			int num_zeros() const;

			/** Returns entry from flattened version of table. */
			Number& operator[](int) const;
			/** Element from table. */
			Number& operator()(int,int);
			Number at(int, int) const;
			bool operator==(const Position&) const;
			SquareIterator squares() const;
			/** Sum of (power of two) of squares. */
			int power_sum() const;
			Number highest() const;
			std::vector<NumPos> highest(int count) const;
			static Position from_str(std::string&&);
	};

	struct PositionHasher
	{
		std::size_t operator()(const Position& pos) const;
	};

	struct MoveResult 
	{
		MoveResultType type;
		Position *new_pos;
	};

	
	struct NumberIdxPop
	{
		Number value;
		int idx; //idx of zero entry, need to be converted to table-idx
	};


	typedef double Probability;
	typedef double Value;
	using boost::container::static_vector;
	/** 
	 * Distribution of positions from a given Position where a random number 
	 * is to be popped.
	 */
	typedef static_vector<std::pair<Probability, Position>, MAX_NUMBERS *2> 
		PositionDistribution;
	class NumberPopper
	{
		private:
#define two_weight 1
			double four_weight;//four_weight:1 chance of popping 4
			mt19937 gen;
			discrete_distribution<> value_dist;
			Probability prob_two, prob_four;

			NumberIdxPop pop(int num_zeros);
			FRIEND_TEST(PositionTest, PopNumber);
		public:
			NumberPopper(double four_weight=.3333333);
			/** Places one random popped number on the given position. */
			void place_one(Position&, ZeroIndices&);
			void place_one(Position&);
			/** Iterates over possible popped values and probabilites. */
			PositionDistribution dist_from(Position&, ZeroIndices&);		
	};

	class PositionIterator
	{
		private:
			const Position* _begin;
			const Position* _end;
		public:
			PositionIterator(const Position*, const Position*);
			const Position* begin();
			const Position* end();
			size_t size();
	};

	class Game 
	{
		private:
			std::vector<Position> positions;
			NumberPopper popper;
		public:
			Game();
			Game(Position&&);
			const Position* current_position();
			MoveResult do_move(UserMove);
			bool is_over();
			PositionIterator history();

			static Game* start_game();
	};


	/******************** ANALYSIS ********************/

	typedef u_int64_t message_key;

	struct Analysis
	{
		Value position_val;
		std::vector<std::array<Value, NUM_DIRECTIONS>> deep_values;
	};



	/********************** UTILS ***********************/



	extern int start_indices[NUM_DIRECTIONS][TABLE_SIZE];
	extern int deltas[NUM_DIRECTIONS];




	GeneralDirection general(MoveDirection);

}

