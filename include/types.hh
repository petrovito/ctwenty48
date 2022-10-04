
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
		Merge merges[TABLE_SIZE -1];

		Number before_merge_segment[TABLE_SIZE];
		Number after_merge_segment[TABLE_SIZE];
		
		bool has_changed;
	};

	struct MoveResultSet
	{
		MoveResultSegment segment_results[TABLE_SIZE];
		bool has_changed;
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

	struct MoveResult 
	{
		MoveResultType type;
	};

	class Position 
	{
		private:
			Bitmap zero_bitmap;
			Number table[TABLE_SIZE][TABLE_SIZE];		

			Position();
			Position(MoveResultSet, MoveDirection);

			MoveResultSet merges_along(MoveDirection);
			MoveResultSet merges_along_at(MoveDirection);
			friend class Game;
		public:
			const Number* get_table(); 
	};


	class Game 
	{
		private:
			Position positions[MAX_MOVES];
			int current_pos_idx;
			Position *current_pos;
		public:
			Game();
			Game(Position);
			const Position* current_position();
			MoveResult do_move(UserMove);
	};




	/********************** UTILS ***********************/



	GeneralDirection general(MoveDirection);

}







