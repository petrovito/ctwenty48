#include "gui.hh"
#include "widgets.hh"
#include <gui_env.hh>
#include <memory>


namespace c20::deps {


	GuiEnv::GuiEnv()
	{
		gui_channel = std::make_unique<gui::GuiMessageChannel>();
		frontend_connector = std::make_unique<gui::FrontendConnector>();
		state_info_handler = std::make_unique<gui::StateInfoHandler>();
		backend_connector = std::make_unique<gui::BackendConnector>();
		window = std::make_unique<gui::C2048Window>();

		frontend_connector->channel = gui_channel.get();
		frontend_connector->handler = state_info_handler.get();
		frontend_connector->init();

		state_info_handler->window = window.get();
		backend_connector->channel = gui_channel.get();
		backend_connector->init();

		window->set_handler(frontend_connector.get());
	}

	ui::UIHandler* GuiEnv::ui_handler() { return backend_connector.get(); }
	

}

