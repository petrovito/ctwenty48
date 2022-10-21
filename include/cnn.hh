#pragma once

#include "types.hh"
#include <cppflow/model.h>
#include <cppflow/tensor.h>

namespace c20::cnn {

	using namespace commons;

	class NeuralEvaluator
	{
		private:
			cppflow::model model;
			NeuralEvaluator(cppflow::model&&);

		public:
			Value evaluate(Position&);
			static NeuralEvaluator* load_from(std::string model_path);
	};

	
	cppflow::tensor tensor_from_pos(Position&);
	Value value_from_tensor(cppflow::tensor&);



}
