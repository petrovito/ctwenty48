#pragma once

#include "oneapi/tbb/concurrent_queue.h"
#include "widgets.hh"
#include <memory>
#include <game_play.hh>
#include <ui.hh>
#include <boost/asio/thread_pool.hpp>


namespace c20::gui {
	using namespace commons;

	enum GuiAction 
	{
		START_GAME
	};

	struct GuiMessage
	{
		GuiAction action;
	};

	class GUI : public ui::UIHandler
	{
		private:
			C2048Window* window;
			
			boost::asio::thread_pool pool;
			tbb::concurrent_bounded_queue<GuiMessage> messages;

			void receive_messages();
		public:
			GUI(core::GamePlayer*, C2048Window*);
			GUI(C2048Window*);

			virtual void set_position(const Position&);
			virtual void game_over();

			void play_a_game();
	};

}
