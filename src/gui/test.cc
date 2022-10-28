#include "types.hh"
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

	TablePanel::TablePanel(nana::form& fm) :
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


}




int main()
{
	using namespace nana;
	form   fm ;                             // Our main window
	fm.caption("My first NANA demo");       // (with this title)
	fm.size({500,400});
	place  fm_place{fm};                    // have automatic layout
	bool   really_quick{false};             // and a default behavior
	label  hello{fm,"Hello World"};         // We put a label on our window
	button btn  {fm,"Quit"};                // and a button
	btn.tooltip( "I will ask first");       // that show a tip upon mouse hover
	group  act  {fm, "Actions"};            // Add a group of "options"
	act.add_option("Quick quickly")         // and two options that control quick behavior
		.events().click([&]()   {   really_quick=true;
				btn.tooltip("Quick quickly");       });
	act.add_option("Ask first")
		.events().click([&]()   {   really_quick=false;
				btn.tooltip("I will ask first");    });
	btn.events().click([&]()               // now the button know how to respond
			{
			if (false)     // not really quick !
			{
			msgbox m(fm,"Our demo", msgbox::yes_no) ;
			m.icon(m.icon_question);
			m << "Are you sure you want to quick?";
			auto response= m();
			if (response != m.pick_yes) return;   // return to the demo
			}
			API::exit();           // or really quick
			});
	act.radio_mode(true);                   // Set "radio mode" (only one option selected)
											// let divide fm into fields to holds the other controls.
											// for example, let split fm into two fields separated by a movable vertical barre.
	fm_place.div("horizontal <table margin=3 weight=400><actions>");
	fm_place["actions"  ]<< hello << btn;        // and place the controls there


	c20::gui::TablePanel bar(fm);
	auto pos = c20::commons::Position::from_str("1111|2222|3333|4444");
	bar.set_position(pos);

	fm_place["table"] << bar;

	/* fm_place[name.c_str()]<< act ; */
	fm_place.collocate();                      // and collocate all in place
	fm.show();
	exec();
}

