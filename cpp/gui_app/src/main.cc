#include "gui.hh"
#include "widgets.hh"
#include <environment.hh>
#include <gui_env.hh>
#include <cnn.hh>
#include <game_play.hh>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>

int main()
{
	spdlog::set_level(spdlog::level::debug);

	c20::deps::EnvSpecs specs{
		.move_selector=c20::deps::MCTS,
		/* .node_eval=c20::deps::Rollout */
		/* .nn_model_path = "neural-net/models/synth_2/" */
	};
	c20::deps::Environment<c20::deps::GuiEnv> env(specs);

	auto pos = c20::commons::Position::from_str("0000|0000|0000|0000");
	auto& state_info = env.ui_env.state_info_handler.get()->state_info;
	state_info.table_pos.modify(pos);

	auto window = env.ui_env.window.get();
	window->do_show();
}

