#include "types.hh"
#include <exception>
#include <selectors.hh>


namespace c20::selectors {


	void MoveSelector::set_position(const Position& _pos)
	{
		pos = _pos;
	}

	RandomSelector::RandomSelector() :
		gen(static_cast<std::uint32_t>(std::time(0))),
		uniform(0, NUM_DIRECTIONS -1)
	{  }

	UserMove RandomSelector::make_move()
	{
		if (pos.is_over()) throw std::exception();
		while (1)
		{
			int num = uniform(gen);
			MoveDirection random_dir = MoveDirection(num);
			auto result = pos.calc_move(random_dir);
			if (result.has_changed) return UserMove{random_dir};
		}
	}

} 

