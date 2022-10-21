#include "types.hh"
#include <cnn.hh>
#include <cppflow/cppflow.h>
#include <cppflow/model.h>
#include <cppflow/tensor.h>
#include <cstdint>
#include <exception>

namespace c20::cnn {

	NeuralEvaluator::NeuralEvaluator(cppflow::model&& _model) :
		model(_model) { }
	
	NeuralEvaluator* NeuralEvaluator::load_from(std::string model_path)
	{
		setenv("TF_CPP_MIN_LOG_LEVEL", "3", true);
		return new NeuralEvaluator((cppflow::model(model_path)));
	}

	Value NeuralEvaluator::evaluate(Position& pos)
	{
		auto input_tensor = tensor_from_pos(pos);
		auto output = model(input_tensor);
		return value_from_tensor(output);	
	}


	cppflow::tensor tensor_from_pos(Position& pos) 
	{
		std::vector<float> values;
		for (Number num: pos.squares()) 
		{
			values.push_back(num);
		}
		cppflow::tensor tensor(values, {1, 16});
		return tensor;
	}

	Value value_from_tensor(cppflow::tensor& tensor)
	{
		return tensor.get_data<float>()[0];
	}


}

