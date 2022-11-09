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
			int current_pos_idx = 0;
			std::vector<PositionWrapper> history;
		public:
			std::array<std::array<std::string, M>, N> label_texts;

			void push_pos_and_adjust(const commons::Position& pos)
			{
				history.push_back({pos});
				adjust(history.size() -1);
			}
			void adjust(int idx)
			{
				current_pos_idx = idx;
				std::pair<int, int> current_pos_coord = {N/2, idx % M};
				{
					int cur_idx = idx;
					auto [i,j] = current_pos_coord;
					bool over = false;
					for (; i < N; i++) {
						for (; j < M; j++) {
							over = cur_idx >= history.size();
							if (over) {
								label_texts[i][j] = "";
							} else {
								label_texts[i][j] = fmt::format("{}", cur_idx);
							}
							cur_idx++;
						}
						j = 0;
					}
				}
				{
					int cur_idx = idx;
					auto [i,j] = current_pos_coord;
					bool over = false;
					for (; i >= 0; i--) {
						for (; j >= 0; j--) {
							over = cur_idx < 0;
							if (over) {
								label_texts[i][j] = "";
							} else {
								label_texts[i][j] = fmt::format("{}", cur_idx);
							}
							cur_idx--;
						}
						j = M-1;
					}
				}
			}
	};
}

