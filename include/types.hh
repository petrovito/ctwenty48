
#pragma once

#include <cstdint>

#define MAX_NUMBERS 16
#define MAX_MOVES 65536


namespace c20::commons {


	typedef uint8_t Number;
	typedef uint16_t Bitmap;
	typedef Number Merge;

	struct MergeSegment 
	{
		Merge first;
		Merge middle;
		Merge last;
		bool saturated;
	};

	struct MergeSet
	{
		//todo
	};

	struct Position 
	{
		Bitmap zeros;
		Bitmap numbers[MAX_NUMBERS];		
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

	

	struct EffectiveMove 
	{
		UserMove user_move;
		Number random_value;
		Bitmap random_pos;
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







