#pragma once

#include "types.hh"

namespace c20::core { class GamePlayer; }

namespace c20::ui {
	using namespace commons;


	class UIHandler
	{
		protected:
			core::GamePlayer *game_player;
		public:
			UIHandler() = default;
			UIHandler(core::GamePlayer*);
			virtual ~UIHandler() = default;
			virtual void set_position(const Position&) = 0;
			virtual void game_over() = 0;
			virtual void analysis_msg(const Analysis&, message_key) = 0;

			virtual void set_game_player(core::GamePlayer*);
	};


	class NoopUI : public UIHandler 
	{
		virtual void set_position(const commons::Position&);
		virtual void game_over();
		virtual void analysis_msg(const Analysis&, message_key);
	};


}
