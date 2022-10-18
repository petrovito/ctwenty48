#include "types.hh"
#include <exception>
#include <selectors.hh>


namespace c20::selectors {


	RandomSelector::RandomSelector() :
		gen(static_cast<std::uint32_t>(std::time(0))),
		uniform(0, NUM_DIRECTIONS)
	{  }

	UserMove RandomSelector::make_move()
	{
		if (pos.is_over()) throw std::exception();
		while (1)
		{
			MoveDirection random_dir = MoveDirection(uniform(gen));
			auto result = pos.calc_move(random_dir);
			if (result.has_changed) return make_move();
		}
	}

} 

