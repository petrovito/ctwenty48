#pragma once

#include "frontend_game.hh"
#include "game_play.hh"
#include <array>
#include <memory>
#include <nana/gui.hpp>
#include <nana/gui/basis.hpp>
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

	class StateInfoHandler;

	class TablePanel : public nana::panel<false>
	{
		private:
			nana::place place;
			std::unique_ptr<nana::label> squares[TABLE_SIZE][TABLE_SIZE];
		public:
			TablePanel(nana::window);
			void set_position(const commons::Position&);
			void set_handler(StateInfoHandler*);
	};


	class MainTab : public nana::panel<false>
	{
		private:
			nana::place place;

			nana::group btn_group;

			nana::button start_game_btn;
			nana::button bot_btn;
			nana::button analyze_btn;

			StateInfoHandler* handler;
		public:
			MainTab(nana::window);
			void set_handler(StateInfoHandler*);
			void game_state_changed(const core::GamePlayerState&);
	};


	class AnalyzePanel : public nana::panel<false>
	{
		private:
			nana::place place;
			nana::label zero_label; //zero depth eval label
			std::vector<std::vector<std::unique_ptr<nana::label>>> table;

			StateInfoHandler* handler;
		public:
			AnalyzePanel(nana::window);
			void set_handler(StateInfoHandler*);
			void update_pos(const commons::Position&);
			void update_texts(const commons::Analysis&);
	};

	class HistoryTab : public nana::panel<false>
	{
		private:
			nana::place place;
			
			std::vector<std::vector<std::unique_ptr<nana::label>>> labels;
			AnalyzePanel analyze_panel;

			StateInfoHandler* handler;
			friend class ControlPanel;
		public:
			HistoryTab(nana::window);
			void set_handler(StateInfoHandler*);
			void update_texts(const GameHistoryView<>&);
	};


	class ControlPanel : public nana::panel<false>
	{
		private:
			nana::place place;
			nana::tabbar<std::string> tab_bar;

			MainTab main_tab;
			HistoryTab history_tab;

			StateInfoHandler* handler;
		public:
			ControlPanel(nana::window);
			void set_handler(StateInfoHandler*);
	};



	class C2048Window : public nana::form
	{

		private:
			nana::place place;
			TablePanel table_panel;
			ControlPanel control_panel;

			StateInfoHandler* handler;
		public:
			C2048Window();

			void set_handler(StateInfoHandler*);
			void do_show();
	};
		



}
