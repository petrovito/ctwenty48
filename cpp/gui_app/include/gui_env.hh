#pragma once

#include "ui.hh"
#include <gui.hh>
#include <memory>


namespace c20::deps {

	class GuiEnv
	{
		public:
			std::unique_ptr<gui::C2048Window> window;
			std::unique_ptr<gui::GuiMessageChannel> gui_channel;
			std::unique_ptr<gui::FrontendConnector> frontend_connector;
			std::unique_ptr<gui::BackendConnector> backend_connector;
			std::unique_ptr<gui::StateInfoHandler> state_info_handler;
		public:
			void instantiate_beans();
			void wire_beans(core::GamePlayer*);
			ui::UIHandler* ui_handler();
	};

}
