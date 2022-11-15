#pragma once

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <types.hh>

namespace c20::selectors {
	using namespace commons;


	class MoveSelector
	{
		protected:
			Position pos;
		public:
			virtual void set_position(const Position&);
			virtual UserMove make_move() = 0;
			virtual Analysis analyze(const Position&);
	};

	
	class RandomSelector : public MoveSelector 
	{
		private:
			boost::random::mt19937 gen;
			boost::random::uniform_int_distribution<> uniform;
		public:
			RandomSelector();
			virtual UserMove make_move();
	};

}

