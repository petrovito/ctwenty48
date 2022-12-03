#include "search.hh"
#include "types.hh"
#include <cnn.hh>
#include <cppflow/cppflow.h>
#include <cppflow/model.h>
#include <cppflow/tensor.h>
#include <cstdint>
#include <exception>
#include <vector>

namespace c20::cnn {

	NeuralEvaluator::NeuralEvaluator(cppflow::model&& _model) :
		model(_model) { }
	
	NeuralEvaluator* NeuralEvaluator::load_from(std::string model_path)
	{
		setenv("TF_CPP_MIN_LOG_LEVEL", "3", true);
		return new NeuralEvaluator((cppflow::model(model_path)));
	}

	void NeuralEvaluator::batch_evaluate(
			std::vector<search::UserNode*>& nodes, search::TimeSpan)
	{
		auto input_tensor = tensor_from_pos(nodes);
		auto output = model(input_tensor).get_data<float>();
		for (uint32_t i = 0; i < nodes.size(); i++)
		{
			auto eval = output[i];
			nodes[i]->dist = 
				search::NodeDistribution::const_dist(eval);
		}
	}


	Value NeuralEvaluator::evaluate(const Position& pos)
	{
		auto input_tensor = tensor_from_pos(pos);
		auto output = model(input_tensor);
		return value_from_tensor(output);	
	}


	cppflow::tensor tensor_from_pos(const Position& pos) 
	{
		std::vector<float> values;
		for (Number num: pos.squares()) 
		{
			values.push_back(num);
		}
		cppflow::tensor tensor(values, {1, 16});
		return tensor;
	}



	cppflow::tensor tensor_from_pos(std::vector<search::UserNode*>& nodes)
	{
		std::vector<float> values;
		for (auto user_node: nodes)
		{
			for (Number num: user_node->pos.squares()) 
			{
				values.push_back(num);
			}
		}
		cppflow::tensor tensor(values, {(long)nodes.size(), 16});
		return tensor;
	}



	Value value_from_tensor(cppflow::tensor& tensor)
	{
		return tensor.get_data<float>()[0];
	}


}

