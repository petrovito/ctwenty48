#include <environment.hh>


namespace c20::deps {

	void NoopUiEnv::instantiate_beans() 
	{
		noop_ui = std::make_unique<ui::NoopUI>();
	}

	void NoopUiEnv::wire_beans(core::GamePlayer*) {}

	ui::UIHandler* NoopUiEnv::ui_handler() { return noop_ui.get(); }

}

