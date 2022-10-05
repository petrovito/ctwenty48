#include <types.hh>


namespace c20::commons {

//start Game class

	Game::Game() :
		current_pos_idx(0)
	{
		//todo
	}


	Game::Game(Position pos)
	{
		//todo
	}


	MoveResult Game::do_move(UserMove user_move) 
	{
		auto dir = user_move.direction;
		auto effect = current_pos->calc_move(dir);
		if (!effect.has_changed) return MoveResult{.type=INVALID};
		positions[++current_pos_idx] = Position(effect, dir);
		current_pos = positions + current_pos_idx;
		//todo add popup
		return MoveResult{SUCCES, current_pos};
	}


	const Position* Game::current_position() 
	{
		return this->current_pos;
	}

	GeneralDirection general(MoveDirection direction) 
	{
		return GeneralDirection(direction >> 1);
	}

//start MoveResultSegment class


	MoveResultSegment::MoveResultSegment() :
		new_segment()  
	{}

	void MoveResultSegment::push_back(Number num, int view_idx) 
	{
		//set has_changed to true if num is changing idx or is merging
		has_changed |= 
			view_idx != before_non_zero || current_merge_candidate == num;
		if (current_merge_candidate == num) //merge current num to last one 
		{
			new_segment[before_non_zero] = num +1; //increment merged entry
			current_merge_candidate = 0; //unset merge candidate
		}
		else
		{
			new_segment[before_non_zero++] = num; //append new entry
			current_merge_candidate = num; //set merge candidate
		}
	}

	Number& MoveResultSegment::operator[](int idx)
	{
		return new_segment[idx];
	}

//start MoveResultSet class

	MoveResultSegment& MoveResultSet::operator[](int idx)
	{
		return segment_results[idx];
	}

//start Position class

	//Mostly inverse of calc_move segments. See comments there.
	Position::Position(MoveResultSet move_result, MoveDirection dir)
	{
		auto delta = deltas[dir];
		for (int segment = 0; segment < TABLE_SIZE; segment++)
		{
			auto idx = start_indices[dir][segment];
			for (int view_idx = 0; view_idx < TABLE_SIZE; view_idx++) 
			{
				(*this)[idx] = move_result[segment][view_idx];	
				idx += delta;
			}
		}
	}

	Number& Position::operator[](int index) 
	{
		return ((Number*)(table))[index];
	}

	MoveResultSegment Position::calc_move_segment(MoveDirection dir, int segment) 
	{
		//get a view of the segment along the direction
		//fx. along RIGHT view starts at right outermost entry 
		//(start_indices) and goes leftwards (delta)
		auto idx = start_indices[dir][segment];
		auto delta = deltas[dir];
		MoveResultSegment result;
		//view_idx: idx of the entry in the segment wrt the view/direction
		for (int view_idx = 0; view_idx < TABLE_SIZE; view_idx++) 
		{
			auto table_entry = (*this)[idx];	
			if (table_entry != 0)
			{
				result.push_back(table_entry, view_idx);
			}
			idx += delta;
		}
		return result;
	}

	MoveResultSet Position::calc_move(MoveDirection dir) 
	{
		MoveResultSet result;
		for (int segment = 0; segment < TABLE_SIZE; segment++)
		{
			auto segment_result = calc_move_segment(dir, segment);
			result.segment_results[segment] = segment_result;
			result.has_changed |= segment_result.has_changed;
		}
		return result;
	}

//start utils
	
	/**
	 * Start indices for (Direction, Segment pair)
	 */
	int start_indices[][TABLE_SIZE] = 
		{
			{0,1,2,3},     //UP
			{12,13,14,15}, //DOWN
			{0,4,8,12},    //LEFT
			{3,7,11,15},   //RIGHT
		};

	/**
	 * Deltas for Direction.
	 */
	int deltas[] = { -4, 4, -1, 1 };




}


