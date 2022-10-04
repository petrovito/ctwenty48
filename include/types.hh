
#pragma once

#include <cstdint>

#define MAX_NUMBERS 16
#define MAX_MOVES 65536
#define TABLE_SIZE 4


namespace c20::commons {


	typedef uint8_t Number;
	typedef uint16_t Bitmap;
	typedef Number Merge;

	struct MoveResultSegment 
	{
		public:
			Number new_segment[TABLE_SIZE];
			
			bool has_changed;
			int before_non_zero = 0;

			MoveResultSegment(); 
			void push_back(Number, int);
			Number& operator[](int);
		private:
			Number current_merge_candidate = 0;
	};

	/**
	 * Set of MoveResultSegment for all segments (along direction).
	 */
	struct MoveResultSet
	{
		MoveResultSegment segment_results[TABLE_SIZE];
		bool has_changed;
		MoveResultSegment& operator[](int);
	};

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


	struct UserMove 
	{
		MoveDirection direction;	
	};

	
	struct RandomPopup
	{
		Number random_value;
		Bitmap random_pos;
	};

	struct EffectiveMove 
	{
		UserMove user_move;
		RandomPopup random_popup;
	};

	enum MoveResultType 
	{
		SUCCES,
		INVALID,
		GAME_OVER,
	};

	class Position 
	{
		private:
			//Bitmap zero_bitmap;
			Number table[TABLE_SIZE][TABLE_SIZE];		

			Position(MoveResultSet, MoveDirection);

			/**
			 * A segment is a row/column.
			 * More precisely is a view of that, i.e. indexing might reversed
			 * if direction is DOWN or RIGHT. 
			 * See also: start_indices, deltas.
			 */
			MoveResultSegment calc_move_segment(MoveDirection, int);

			/**
			 * Calculates views for move along direction.
			 */
			MoveResultSet calc_move(MoveDirection);

			/**
			 * Returns entry from flattened version of table.
			 */
			Number& operator[](int);
			friend class Game;
		public:
			Position();
			const Number* get_table(); 
	};


	struct MoveResult 
	{
		MoveResultType type;
		Position *new_pos;
	};

	class NumberPopper
	{
		//todo attributes: strategies, random? probs..
	};

	class Game 
	{
		private:
			Position positions[MAX_MOVES];
			int current_pos_idx;
			Position *current_pos;
			NumberPopper popper;
			
		public:
			Game();
			Game(Position);
			const Position* current_position();
			MoveResult do_move(UserMove);
	};




	/********************** UTILS ***********************/


	extern int start_indices[NUM_DIRECTIONS][TABLE_SIZE];
	extern int deltas[NUM_DIRECTIONS];




	GeneralDirection general(MoveDirection);

}

