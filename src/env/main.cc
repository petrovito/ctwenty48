#include <boost/filesystem/file_status.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <env/environment.hh>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
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
		;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, args, desc), vm);
	po::notify(vm);    



	cout<<"Running ctwenty48."<<endl;



	if (!vm.count("random")) 
	{
		cout<<"Non-random not yet supported."<<endl;
		return 1;
	}

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

	c20::core::Environment env;
	env.play_games_and_quit(vm["num"].as<int>(), log_path);
}



