#include <algorithm>
#include <boost/container/static_vector.hpp>
#include <boost/random/discrete_distribution.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <tuple>
#include <types.hh>

#include <gtest/gtest_prod.h>
#include <utility>
#include <vector>

using boost::random::uniform_int_distribution;

namespace c20::commons {

	volatile bool shutting_down = false;

//start Game class

	Game::Game()
	{ 
		positions.push_back(Position());
	}


	Game::Game(Position&& pos)
	{
		positions.push_back(pos);
	}


	MoveResult Game::do_move(UserMove user_move) 
	{
		auto dir = user_move.direction;
		auto effect = positions.back().calc_move(dir);
		//calc new pos without Popup
		if (!effect.has_changed) return MoveResult{.type=INVALID};
		auto [new_pos, zeros] = effect.calc_pos_zeros_pair();
		//add popup
		if (zeros.size()) {
			popper.place_one(new_pos, zeros);
		}	
		positions.push_back(new_pos);
		return MoveResult{SUCCES, &positions.back()};
	}

	bool Game::is_over()
	{
		return positions.back().is_over();
	}

	const Position* Game::current_position() 
	{
		return &positions.back();
	}


	PositionIterator Game::history() 
	{
		return PositionIterator(positions.begin().base(), positions.end().base());
	}


	Game* Game::start_game()
	{
		Game *game = new Game();
		game->popper.place_one(game->positions.back());
		game->popper.place_one(game->positions.back());
		return game;
	}


//start SquareIterator

	const Number* SquareIterator::begin() { return _begin; }
	const Number* SquareIterator::end() { return _end; }
	SquareIterator::SquareIterator(const Number* begin, const Number* end) :
		_begin(begin), _end(end) {}

//start PositionIterator

	const Position* PositionIterator::begin() { return _begin; }
	const Position* PositionIterator::end() { return _end; }
	PositionIterator::PositionIterator(const Position* begin, const Position* end) :
		_begin(begin), _end(end) {}
	size_t PositionIterator::size() { return _end-_begin; }


//start MoveResultSegment class


	MoveResultSegment::MoveResultSegment() :
		new_segment()  {}

	void MoveResultSegment::push_back(Number num, int view_idx) 
	{
		//set has_changed to true if num is changing idx or is merging
		has_changed |= 
			view_idx != non_zero || current_merge_candidate == num;
		if (current_merge_candidate == num) //merge current num to last one 
		{
			new_segment[non_zero -1] = num +1; //increment merged entry
			current_merge_candidate = 0; //unset merge candidate
		}
		else
		{
			new_segment[non_zero++] = num; //append new entry
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

	Number& Position::operator[](int index) const
	{
		return ((Number*)(table))[index];
	}


	Number& Position::operator()(int row, int column) 
	{
		return table[row][column];
	}

	Number Position::at(int row, int column) const
	{
		return table[row][column];
	}
	
	bool Position::operator==(const Position& other) const
	{
		return memcmp(this->table, other.table, NUM_SQUARES * sizeof(Number)) == 0;
	}

	MoveResultSegment Position::calc_move_segment(MoveDirection dir, int segment) const
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
			auto table_entry = (*const_cast<Position*>(this))[idx];	
			if (table_entry != 0)
			{
				result.push_back(table_entry, view_idx);
			}
			idx += delta;
		}
		return result;
	}

	MoveResultSet Position::calc_move(MoveDirection dir) const 
	{
		MoveResultSet result{.has_changed=false, .dir=dir};
		for (int segment = 0; segment < TABLE_SIZE; segment++)
		{
			auto segment_result = calc_move_segment(dir, segment);
			result.segment_results[segment] = segment_result;
			result.has_changed |= segment_result.has_changed;
		}
		return result;
	}

	bool Position::is_over() const
	{
		//check for horizontal merge possibilities OR zeros
		for (int i = 0; i < TABLE_SIZE; i++)
		{
			for (int j = 0; j < TABLE_SIZE -1; j++)
			{
				if (table[i][j] == table[i][j +1] || table[i][j] == 0) 
					return false;
			}
			if (table[i][TABLE_SIZE -1] == 0) return false;
		}
		//check for vertical merge possibilities (not zeroes this time)
		for (int i = 0; i < TABLE_SIZE; i++)
		{
			for (int j = 0; j < TABLE_SIZE -1; j++)
			{
				if (table[j][i] == table[j +1][i]) 
					return false;
			}
		}
		return true;
	}

	int Position::num_zeros() const
	{
		int zeros = 0;
		for (int i = 0; i < TABLE_SIZE*TABLE_SIZE; i++)
		{
			if ((*const_cast<Position*>(this))[i] == 0) zeros++;
		}
		return zeros;
	}

	int Position::power_sum() const
	{
		int sum = 0;
		for (int i = 0; i < NUM_SQUARES; i++) {
			sum += 1 << (*this)[i]; //TODO this is not precise
		}
		return sum;
	}

	Number Position::highest() const
	{
		return *std::max_element((Number*)table, (Number*)table +NUM_SQUARES);
	}

	uint8_t Position::highest_idx() const
	{
		return std::distance((Number*) table,
				std::max_element(
					(Number*)table, (Number*)table +NUM_SQUARES)
				);
	}

	std::vector<NumPos> Position::highest(int count) const
	{
		std::array<NumPos, NUM_SQUARES> squares;
		auto loc_table = (Number*) table;
		for (int i = 0; i < NUM_SQUARES; i++) 
		{
			squares[i] = {.idx=i, .num=loc_table[i]};
		}

		std::nth_element(squares.begin(), squares.begin() +count,
				squares.end(), [] (auto& sq1, auto& sq2) {
					return sq1.num > sq2.num;
				});
		
		std::vector<NumPos> highests(count);

		for (int i = 0; i < count; i++)
			highests[i] = squares[i];

		std::sort(highests.begin(), highests.end(), 
				[] (auto& sq1, auto& sq2) {
					return sq1.num > sq2.num;
				});

		return highests;
	}

	int Position::count_above(int threshold) const
	{
		auto loc_table = (Number*) table;
		return std::count_if(loc_table, loc_table +NUM_SQUARES,
				[threshold] (auto& num) {return num > threshold;});
	}
	
	Position Position::from_str(std::string &&table_str)
	{
		Position pos;
		int col = 0;
		int row = 0;
		for (char const c: table_str)
		{
			if (c == '|') {
				row++;
				col=0;
			}
			else pos.table[row][col++] = c - '0';
		}
		return pos;
	}

	SquareIterator Position::squares() const
	{
		return SquareIterator((Number*) table, (Number*) table + NUM_SQUARES);
	}


//NumberPopper class

	NumberPopper::NumberPopper(double four_weight) :
		four_weight(four_weight),
		gen(static_cast<std::uint32_t>(std::time(0))),
		value_dist(discrete_distribution<>{1, four_weight})
	{ 
		prob_two = two_weight / (two_weight + four_weight);
		prob_four = four_weight / (two_weight + four_weight);
	}

	NumberIdxPop NumberPopper::pop(int num_zeros)
	{
		uniform_int_distribution<> uniform(0, num_zeros -1);
		auto idx = uniform(gen);
		auto value = Number(value_dist(gen) +1);
		return NumberIdxPop{value, idx};
	}

	
	void NumberPopper::place_one(Position& pos, ZeroIndices& zeros) 
	{
		NumberIdxPop popped = pop(zeros.size());
		int table_idx = zeros[popped.idx];
		pos[table_idx] = popped.value;
	}

	void NumberPopper::place_one(Position& pos)
	{
		ZeroIndices zeros;
		for (int i = 0; i < NUM_SQUARES; i++)
		{
			if (pos[i] == 0) zeros.push_back(i);
		}
		place_one(pos, zeros);
	}

	PositionDistribution NumberPopper::dist_from(
			Position& pos, ZeroIndices& zeros)
	{
		int num_zeros = zeros.size();
		Probability two_chance = prob_two / num_zeros;
		Probability four_chance = prob_four / num_zeros;
		PositionDistribution dist;
		for (auto zero_idx: zeros)
		{
			//for (Number popped_num: {1,2})
			{ //pop two
				Position new_pos = pos; //copy original
				new_pos[zero_idx] = 1;
				dist.push_back({two_chance, new_pos});
			}
			{ //pop four
				Position new_pos = pos; //copy original
				new_pos[zero_idx] = 2;
				dist.push_back({four_chance, new_pos});
			}
		}
		return dist;
	}

//PositionHasher

	size_t PositionHasher::operator()(const Position &pos) const
	{
		size_t hash = 0;
		for (int i = 0; i < NUM_SQUARES; i++)
		{
			hash = hash *31 + pos[i];
		}
		return hash;
	}


//start utils
	
	//Mostly inverse of calc_move segments. See comments there.
	std::tuple<Position, ZeroIndices>  MoveResultSet::calc_pos_zeros_pair()
	{
		Position new_pos;
		ZeroIndices zeros;
		auto delta = deltas[dir];
		for (int segment = 0; segment < TABLE_SIZE; segment++)
		{
			auto idx = start_indices[dir][segment];
			for (int view_idx = 0; view_idx < TABLE_SIZE; view_idx++) 
			{
				Number num = (*this)[segment][view_idx];	
				new_pos[idx] = num;
				if (num == 0) 
				{
					zeros.push_back(idx);
				}
				idx += delta;
			}
		}
		return std::make_tuple(new_pos, zeros);
	}

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
	int deltas[] = { 4, -4, 1, -1 };




}


