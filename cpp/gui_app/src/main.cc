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

	c20::deps::EnvSpecs specs{.nn_model_path = "neural-net/models/v1/"};
	c20::deps::Environment<c20::deps::GuiEnv> env(specs);

	auto pos = c20::commons::Position::from_str("0000|0000|0000|0000");
	auto window = env.ui_env.window.get();
	window->set_position(pos);
	window->do_show();
}

