#include "game_play.hh"
#include "gui.hh"
#include "widgets.hh"
#include <gui_env.hh>
#include <memory>


namespace c20::deps {

	void GuiEnv::instantiate_beans()
	{
		gui_channel = std::make_unique<gui::GuiMessageChannel>();
		frontend_connector = std::make_unique<gui::FrontendConnector>();
		state_info_handler = std::make_unique<gui::StateInfoHandler>();
		backend_connector = std::make_unique<gui::BackendConnector>();
		window = std::make_unique<gui::C2048Window>();
	}

	void GuiEnv::wire_beans(core::GamePlayer* game_player)
	{
		frontend_connector->channel = gui_channel.get();
		frontend_connector->handler = state_info_handler.get();
		frontend_connector->init();

		state_info_handler->window = window.get();
		state_info_handler->connector = frontend_connector.get();
		backend_connector->channel = gui_channel.get();
		backend_connector->game_player = game_player;
		backend_connector->init();

		window->set_handler(state_info_handler.get());
	}

	ui::UIHandler* GuiEnv::ui_handler() { return backend_connector.get(); }
	

}

