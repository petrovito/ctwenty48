#include "gui.hh"
#include "widgets.hh"
#include <env/environment.hh>
#include <cnn.hh>
#include <game_play.hh>

int main()
{
	auto window = std::shared_ptr<c20::gui::C2048Window>(new c20::gui::C2048Window());
	auto pos = c20::commons::Position::from_str("1111|2222|3333|4444");
	window->set_position(pos);

	std::shared_ptr<c20::gui::BackendConnector> gui =
		std::shared_ptr<c20::gui::BackendConnector>(new c20::gui::BackendConnector(window.get()));

	window->set_handler(gui);
	std::shared_ptr<c20::ui::UIHandler> ui = gui;


	auto popper = std::shared_ptr<c20::commons::NumberPopper>(new c20::commons::NumberPopper());
	
	auto node_eval = std::shared_ptr<c20::cnn::NeuralEvaluator>(
			c20::cnn::NeuralEvaluator::load_from("cnn/models/v1/"));
	std::shared_ptr<c20::selectors::MoveSelector> selector =
		std::shared_ptr<c20::search::SearchManager>(
			new c20::search::SearchManager(node_eval.get(), *popper));

	auto game_player = std::shared_ptr<c20::core::GamePlayer>(
			new c20::core::GamePlayer(ui, selector));
	gui->set_game_player(game_player.get());

	c20::core::Environment env{
		selector,
		ui,
		game_player,
		window
	};
	window->do_show();
}

