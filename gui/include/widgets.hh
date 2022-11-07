#pragma once

#include <array>
#include <memory>
#include <nana/gui.hpp>
#include <nana/gui/place.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/form.hpp>
#include <nana/gui/widgets/group.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/panel.hpp>
#include <nana/gui/widgets/tabbar.hpp>
#include <types.hh>
#include <vector>


namespace c20::gui {

	class FrontendConnector;

	class TablePanel : public nana::panel<false>
	{
		private:
			nana::place place;
			std::unique_ptr<nana::label> squares[TABLE_SIZE][TABLE_SIZE];
		public:
			TablePanel(nana::window);
			void set_position(const commons::Position&);
	};


	class MainTab : public nana::panel<false>
	{
		private:
			nana::place place;

			nana::group btn_group;

			nana::button start_btn;

			FrontendConnector* handler;
		public:
			MainTab(nana::window);
			void set_handler(FrontendConnector*);
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

			FrontendConnector* handler;
		public:
			ControlPanel(nana::window);
			void set_handler(FrontendConnector*);
	};



	class C2048Window : public nana::form
	{

		private:
			nana::place place;
			TablePanel table_panel;
			ControlPanel control_panel;

			FrontendConnector* handler;
		public:
			C2048Window();

			void set_handler(FrontendConnector*);
			void do_show();
			void set_position(const commons::Position&);
	};
		



}
