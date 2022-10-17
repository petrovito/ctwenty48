#pragma once

#include "types.hh"

namespace c20::cnn {

	using namespace commons;

	class NeuralEvaluator
	{
		private:
			NeuralEvaluator();
		public:
			Value evaluate(Position&);
			static NeuralEvaluator load_from();
	};

	



}
