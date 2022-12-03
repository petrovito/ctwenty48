#pragma once

#include "types.hh"
#include <fmt/core.h>
#include <utility>
#include <vector>

namespace c20::gui {

	struct PositionWrapper
	{
		commons::Position pos;
	};


	template<int M=10, int N=5>
	class GameHistoryView
	{
		private:
			std::vector<PositionWrapper> history;
			int current_pos_idx = 0;
		public:
			std::pair<int, int> current_pos_coord;
			std::array<std::array<std::string, M>, N> label_texts;

			unsigned height() const {return M;}
			unsigned width() const {return N;}
			size_t size() const {return history.size();}
			unsigned current_idx() const {return current_pos_idx;}
			const PositionWrapper& at(int idx) {return history[idx];}


			void push_pos_and_adjust(const commons::Position& pos)
			{
				history.push_back({pos});
				adjust(history.size() -1);
			}
			void adjust(int idx)
			{
				current_pos_idx = idx;
				current_pos_coord = {N/2, idx % M};

				int cur_idx = idx - 
					current_pos_coord.first *M - current_pos_coord.second;
				for (int i = 0; i < N; i++) {
					for (int j = 0; j < M; j++) {
						if (cur_idx < 0 || (unsigned) cur_idx >= history.size()) {
							label_texts[i][j] = "";
						} else {
							label_texts[i][j] = fmt::format("{}", cur_idx);
						}
						cur_idx++;
					}
				}
			}
	};
}

