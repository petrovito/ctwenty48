#pragma once

#include <array>
#include <nana/gui.hpp>
#include <nana/gui/place.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/group.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/panel.hpp>
#include <nana/gui/widgets/tabbar.hpp>
#include <types.hh>
#include <vector>


namespace c20::gui {


	class TablePanel : public nana::panel<false>
	{
		private:
			nana::place place;
			std::unique_ptr<nana::label> squares[TABLE_SIZE][TABLE_SIZE];
		public:
			TablePanel(nana::window);
			void set_position(commons::Position&);
	};


	class MainTab : public nana::panel<false>
	{
		private:
			nana::place place;

			nana::group btn_group;

			nana::button start_btn;
		public:
			MainTab(nana::window);
	};

	class HistoryTab : public nana::panel<false>
	{
		private:
			nana::place place;
		public:
			HistoryTab(nana::window);
	};


	class ControlPanel : public nana::panel<false>
	{
		private:
			nana::place place;
			nana::tabbar<std::string> tab_bar;

			MainTab main_tab;
			HistoryTab history_tab;

		public:
			ControlPanel(nana::window);

	};

}
