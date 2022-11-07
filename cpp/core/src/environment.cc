#include <environment.hh>


namespace c20::deps {

	NoopUiEnv::NoopUiEnv() : noop_ui(std::make_unique<ui::NoopUI>()) {}
	ui::UIHandler* NoopUiEnv::ui_handler() { return noop_ui.get(); }

}

