#include "game_play.hh"
#include "gui.hh"
#include "search.hh"
#include "types.hh"
#include "ui.hh"
#include <iostream>
#include <memory>
#include <nana/basic_types.hpp>
#include <nana/gui.hpp>                  // always include this
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/group.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/msgbox.hpp>

#include <widgets.hh>
#include <nana/gui/widgets/panel.hpp>
#include <string>

namespace c20::gui {


	int colors[] = {
		0xe6194b,
		0x3cb44b,
		0xffe119,
		0x4363d8,
		0xf58231,
		0x911eb4,
		0x46f0f0,
		0xf032e6,
		0xbcf60c,
		0xfabebe,
		0x008080,
		0xe6beff,
		0x9a6324,
		0xfffac8,
		0x800000,
		0xaaffc3,
		0x808000,
		0xffd8b1,
		0x000075,
		0x808080,
		0xffffff,
		0x000000,
	};

	TablePanel::TablePanel(nana::window fm) :
		nana::panel<false>(fm),
		place(*this)
	{
		place.div("vertical <table weight=400 grid=[4,4]>");
		for (int i = 0; i < TABLE_SIZE; i++)
			for (int j = 0; j < TABLE_SIZE; j++)
			{
				auto& sq = squares[i][j];
				sq.reset(new nana::label{*this});
				sq->text_align(nana::align::center, nana::align_v::center);
				sq->typeface({"", 35});
				place["table"] << *sq;
			}
	}

	void TablePanel::set_position(const commons::Position& pos) 
	{
		for (int i = 0; i < TABLE_SIZE; i++)
			for (int j = 0; j < TABLE_SIZE; j++)
			{
				auto& sq = squares[i][j];
				auto num = pos.at(i, j);
				sq->caption(std::to_string(num));
				sq->bgcolor(nana::color_rgb(colors[num]));
			}
	}

	ControlPanel::ControlPanel(nana::window fm) :
		nana::panel<false>(fm),
		place(*this),
		tab_bar(*this),
		main_tab(*this),
		history_tab(*this)
	{
		place.div(R"(vertical 
			<tab weight=20>
			<tab_frame>
		)");

		place["tab"] << tab_bar;
		place["tab_frame"].fasten(main_tab)
			.fasten(history_tab);

		tab_bar.append("main", main_tab)
			.append("history", history_tab);
		tab_bar.activated(0);

	}

	MainTab::MainTab(nana::window fm) : 
		nana::panel<false>(fm),
		place(*this),
		btn_group(*this, "Game"),
		start_btn(btn_group, "Start game")
	{
		place.div(R"( 
			vertical 
			<all   weight=160 gap=3 margin=25>
			<rest>
		 )");
		place["all"] << btn_group;

		btn_group.div(R"(
			<weight=10>
		    <buttons weight=125 gap=5 margin=20>
			<weight=10>
		)");
		btn_group["buttons"] << start_btn;

		start_btn.events().click([] (auto& var) {std::cout<<"CLICKED"<<std::endl;});
	}

	HistoryTab::HistoryTab(nana::window fm) :
		nana::panel<false>(fm),
		place(*this)
	{}


	C2048Window::C2048Window() : 
		place(*this),
		table_panel(*this),
		control_panel(*this)
	{
		caption("C2048");
		size({900, 400});

		place.div("horizontal <table margin=3 weight=400><control>");
		place["table"] << table_panel;
		place["control"] << control_panel;
	}


	void C2048Window::do_show() 
	{
		place.collocate();
		show();
		nana::exec();
	}


	void C2048Window::set_position(const commons::Position& pos) 
	{
		table_panel.set_position(pos);
	}


	void C2048Window::set_handler(std::shared_ptr<GUI>& _handler)
	{
		handler = _handler;
		control_panel.set_handler(_handler);
	}

	void ControlPanel::set_handler(std::shared_ptr<GUI>& _handler)
	{
		handler = _handler;
		main_tab.set_handler(_handler);
	}

	void MainTab::set_handler(std::shared_ptr<GUI>& _handler)
	{
		handler = _handler;
	}


}


#include <env/environment.hh>
#include <cnn.hh>
#include <game_play.hh>

int main()
{

	auto window = std::shared_ptr<c20::gui::C2048Window>();
	auto pos = c20::commons::Position::from_str("1111|2222|3333|4444");
	window->set_position(pos);

	std::shared_ptr<c20::gui::GUI> gui =
		std::shared_ptr<c20::gui::GUI>(new c20::gui::GUI(window.get()));

	window->set_handler(gui);
	std::shared_ptr<c20::ui::UIHandler> ui = gui;


	auto popper = std::shared_ptr<c20::commons::NumberPopper>();
	
	auto node_eval = std::shared_ptr<c20::cnn::NeuralEvaluator>(
			c20::cnn::NeuralEvaluator::load_from("cnn/models/v1/"));
	std::shared_ptr<c20::core::MoveSelector> selector =
		std::shared_ptr<c20::search::SearchManager>(
			new c20::search::SearchManager(node_eval.get(), *popper));

	auto game_player = std::shared_ptr<c20::core::GamePlayer>(
			new c20::core::GamePlayer(ui, selector));

	c20::core::Environment env{
		selector,
		ui,
		game_player,
		window
	};
	window->do_show();
}

