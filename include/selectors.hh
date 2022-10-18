#pragma once

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <game_play.hh>

namespace c20::selectors {

	using namespace commons;
	
	class RandomSelector : public core::MoveSelector 
	{
		private:
			boost::random::mt19937 gen;
			boost::random::uniform_int_distribution<> uniform;
		public:
			RandomSelector();
			virtual UserMove make_move();
	};

}

