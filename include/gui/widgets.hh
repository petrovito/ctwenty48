#pragma once

#include <array>
#include <nana/gui.hpp>
#include <nana/gui/place.hpp>
#include <nana/gui/widgets/label.hpp>

#include <nana/gui/widgets/panel.hpp>
#include <types.hh>
#include <vector>


namespace c20::gui {


	class TablePanel : public nana::panel<false>
	{
		private:
			nana::place place;
			std::unique_ptr<nana::label> squares[TABLE_SIZE][TABLE_SIZE];
		public:
			TablePanel(nana::form&);
			void set_position(commons::Position&);
	};

}
