#include "types.hh"
#include <iostream>
#include <nana/basic_types.hpp>
#include <nana/gui.hpp>                  // always include this
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/group.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/msgbox.hpp>

#include <gui/widgets.hh>
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
		place.collocate();
	}

	void TablePanel::set_position(commons::Position& pos) 
	{
		for (int i = 0; i < TABLE_SIZE; i++)
			for (int j = 0; j < TABLE_SIZE; j++)
			{
				auto& sq = squares[i][j];
				auto num = pos(i, j);
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


}




int main()
{
	using namespace nana;
	form   fm ;                             // Our main window
	fm.caption("c2048");       // (with this title)
	fm.size({900,400});
	place  fm_place{fm};                    // have automatic layout

	fm_place.div("horizontal <table margin=3 weight=400><actions>");

	c20::gui::TablePanel bar(fm);
	auto pos = c20::commons::Position::from_str("1111|2222|3333|4444");
	bar.set_position(pos);

	c20::gui::ControlPanel control(fm);

	fm_place["table"] << bar;
	fm_place["actions"] << control;

	fm_place.collocate();                      // and collocate all in place
	fm.show();
	exec();
}

