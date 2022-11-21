#pragma once

#include "search.hh"
#include "types.hh"
#include <cppflow/model.h>
#include <cppflow/tensor.h>
#include <vector>

namespace c20::cnn {

	using namespace commons;

	class NeuralEvaluator : public search::NodeEvaluator
	{
		private:
			cppflow::model model;
			NeuralEvaluator(cppflow::model&&);

		public:
			Value evaluate(const Position&) override;
			void batch_evaluate(std::vector<search::UserNode*>&, 
					search::TimeSpan) override;
			static NeuralEvaluator* load_from(std::string model_path);
	};

	
	cppflow::tensor tensor_from_pos(const Position&);
	cppflow::tensor tensor_from_pos(std::vector<search::UserNode*>&);
	Value value_from_tensor(cppflow::tensor&);



}
