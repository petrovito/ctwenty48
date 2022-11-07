#include "gui.hh"
#include "widgets.hh"
#include <env/environment.hh>
#include <gui_env.hh>
#include <cnn.hh>
#include <game_play.hh>

int main()
{
	c20::deps::EnvSpecs specs{.nn_model_path = "cnn/models/v1/"};
	c20::deps::Environment<c20::deps::GuiEnv> env(specs);

	auto pos = c20::commons::Position::from_str("1111|2222|3333|4444");
	auto window = env.ui_env.window.get();
	window->set_position(pos);

	window->do_show();
}

