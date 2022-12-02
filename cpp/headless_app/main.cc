#include "cnn.hh"
#include "game_play.hh"
#include "search.hh"
#include "selectors.hh"
#include "types.hh"
#include "ui.hh"
#include <boost/filesystem/file_status.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <environment.hh>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <memory>
#include <spdlog/spdlog.h>
#include <string>

namespace po = boost::program_options;
using namespace std;

int main(int argc, char** args) {
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("random", "random games")
		("num", po::value<int>()->default_value(1), "number of games to play")
		("log-path", po::value<string>()->default_value("/dev/null"), "game logs path")
		("model-path", po::value<string>()->default_value("neural-net/models/v1"), "model path")
		("mce", "Monte Carlo estimator")
		("mcts", "Monte Carlo Tree Search")
		("params", po::value<string>()->default_value(""), "Monte Carlo Tree Search params file location")
		("rollout", "Rollout node evaluator")
		;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, args, desc), vm);
	po::notify(vm);    


	c20::deps::EnvSpecs specs;


	if (vm.count("random")) {
		spdlog::info("Using random selector.");
		specs.move_selector = c20::deps::RandomSelector;
	} else if (vm.count("mce")) {
		spdlog::info("Using MCE.");
		specs.move_selector = c20::deps::MCE;
	} else if (vm.count("mcts")) {
		spdlog::info("Using MCTS.");
		specs.move_selector = c20::deps::MCTS;
		specs.node_eval = c20::deps::Static;
		if (vm.count("params"))
			specs.mcts_param_path = vm["params"].as<string>();
	} else {
		spdlog::info("Using search manager.");
		if (vm.count("rollout")) {
			spdlog::info("Using rollout evaluator.");
			specs.node_eval = c20::deps::Rollout;
		} else {
			spdlog::info("Using neural net evaluator.");
			specs.move_selector = c20::deps::SearchManager;
			specs.nn_model_path = vm["model-path"].as<string>();
		}
	}

	c20::deps::Environment env(specs);


	auto log_path = vm["log-path"].as<string>();
	boost::filesystem::path p(log_path);
	if (
			(p.has_parent_path() && !exists(p.parent_path())) ||
			(boost::filesystem::is_directory(p))
	   )
	{
		cout<<"Invalid filename: "<<log_path<<endl;
		return 1;
	}

	env.play_games(vm["num"].as<int>(), log_path);
}



