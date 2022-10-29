#include "cnn.hh"
#include "game_play.hh"
#include "search.hh"
#include "selectors.hh"
#include "types.hh"
#include "ui.hh"
#include <boost/filesystem/file_status.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <env/environment.hh>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <memory>
#include <string>

namespace po = boost::program_options;
using namespace std;

int main(int argc, char** args) {
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("random", "random games")
		("num", po::value<int>()->default_value(1), "number of games to play")
		("log-path", po::value<string>()->default_value("/dev/stdout"), "game logs path")
		("model-path", po::value<string>()->default_value("cnn/models/v1"), "model path")
		;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, args, desc), vm);
	po::notify(vm);    


	cout<<"Running ctwenty48."<<endl;


	shared_ptr<c20::selectors::MoveSelector> move_selector;
	shared_ptr<c20::ui::UIHandler> ui;
	shared_ptr<c20::core::GamePlayer> game_player;


	if (!vm.count("random")) 
	{
		c20::search::NodeEvaluator* node_eval =
			c20::cnn::NeuralEvaluator::load_from(vm["model-path"].as<string>());
		c20::commons::NumberPopper popper;
		move_selector.reset(new c20::search::SearchManager(node_eval, popper));
		ui.reset(new c20::ui::NoopUI());
	}
	else 
	{
		move_selector.reset(new c20::selectors::RandomSelector());
		ui.reset(new c20::ui::NoopUI());
	}
	game_player.reset(new c20::core::GamePlayer(ui, move_selector));

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

	c20::core::Environment env{.move_selector=move_selector, .ui=ui, .game_player=game_player};
	env.play_games_and_quit(vm["num"].as<int>(), log_path);
}



