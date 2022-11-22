#pragma once
#include "game_logger.hh"
#include "game_play.hh"
#include "mc_estimate.hh"
#include "mcts.hh"
#include "rollout_eval.hh"
#include "search.hh"
#include "ui.hh"
#include <memory>
#include <types.hh>
#include <cnn.hh>


namespace c20::deps {
	
	
	enum MoveSelector { SearchManager, RandomSelector, MCE, MCTS };
	enum NodeEvaluator { CNN, Rollout };
	enum UI { NONE, C2048 };


	/**
	 * List of specs for dependencies/beans to use, when injecting
	 * dependencies.
	 */
	struct EnvSpecs
	{
		MoveSelector move_selector = SearchManager;
		NodeEvaluator node_eval = CNN;
		UI ui = NONE;

		std::string nn_model_path = "";
	};

	class NoopUiEnv
	{
		private:
			std::unique_ptr<ui::NoopUI> noop_ui;
		public:
			void wire_beans(core::GamePlayer*);
			void instantiate_beans();
			ui::UIHandler* ui_handler();
	};

	/**
	 * Class responsible for instantiating components, and wire links.
	 * Basically, entry point into application.
	 */
	template<typename UiEnv>
	class Environment
	{
		public:
			EnvSpecs specs;

			//beans
			selectors::MoveSelector* move_selector;
			std::unique_ptr<selectors::RandomSelector> random_selector;
			std::unique_ptr<search::SearchManager> search_manager;
			std::unique_ptr<search::MonteCarloEstimator> mc_estimator;

			std::unique_ptr<mcts::MCTS> mcts;
			std::unique_ptr<mcts::NodeContainer> mcts_nodes;

			search::NodeEvaluator* node_eval;
			std::unique_ptr<search::RolloutEvaluator> rollout_eval;
			std::unique_ptr<cnn::NeuralEvaluator> neural_eval;
			std::unique_ptr<commons::NumberPopper> number_popper;
			std::unique_ptr<search::NodeContainer> node_container;
			std::unique_ptr<search::GraphSearcher> graph_searcher;
			std::unique_ptr<search::GraphEvaluator> graph_evaluator;

			UiEnv ui_env;

			std::unique_ptr<core::GamePlayer> game_player;

			void instantiate_beans()
			{
				number_popper = std::make_unique<commons::NumberPopper>();
				game_player = std::make_unique<core::GamePlayer>();

				switch (specs.move_selector) {
					case RandomSelector:
						random_selector = std::make_unique<selectors::RandomSelector>();
						move_selector = random_selector.get();
						break;
					case SearchManager:
						search_manager = std::make_unique<search::SearchManager>();
						move_selector = search_manager.get();
						node_container = std::make_unique<search::NodeContainer>();
						graph_searcher = std::make_unique<search::GraphSearcher>();
						graph_evaluator = std::make_unique<search::GraphEvaluator>();
						switch (specs.node_eval) {
						case CNN:
							neural_eval = std::unique_ptr<cnn::NeuralEvaluator>(
									cnn::NeuralEvaluator::load_from(specs.nn_model_path));
							node_eval = neural_eval.get();
							break;
						case Rollout:
							rollout_eval = std::make_unique<search::RolloutEvaluator>();
							node_eval = rollout_eval.get();
							break;
						}
						break;
					case MCE:
						mc_estimator = std::make_unique<search::MonteCarloEstimator>();
						move_selector = mc_estimator.get();
						break;
					case MCTS:
						mcts = std::make_unique<mcts::MCTS>();
						mcts_nodes = std::make_unique<mcts::NodeContainer>();
						move_selector = mcts.get();
						break;
				}

				ui_env.instantiate_beans();
			}

			void wire_beans()
			{
				switch (specs.move_selector) {
					case RandomSelector: break; //pass
					case SearchManager:
						search_manager->node_container = node_container.get();
						search_manager->graph_searcher = graph_searcher.get();
						search_manager->graph_evaluator = graph_evaluator.get();
						search_manager->node_eval = node_eval;

						graph_searcher->popper = number_popper.get();
						graph_searcher->node_container = node_container.get();

						if (specs.node_eval == Rollout)
							rollout_eval->popper = number_popper.get();

						break;
					case MCE:
						mc_estimator->popper(number_popper.get());
						break;
					case MCTS:
						mcts->number_popper = number_popper.get();
						mcts->node_container = mcts_nodes.get();
						break;
				}

				game_player->move_selector = move_selector;
				game_player->ui = ui_env.ui_handler();

				ui_env.wire_beans(game_player.get());
			}
		public:
			Environment(const EnvSpecs& _specs) :
				specs(_specs)
			{
				instantiate_beans();
				wire_beans();
			}

			//TODO this function doesn't belong here. Move it!
			void play_games(int num_games, const std::string& path)
			{
				std::unique_ptr<misc::GameLogger> logger(path.empty() ?
						nullptr : new misc::GameLogger(path));
				for (int i = 0; i < num_games; i++)
				{
					auto game = game_player->play_a_game();
					if (logger.get()) logger->log_game(*game);
				}
			}

	};

}

