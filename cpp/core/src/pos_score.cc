#include "types.hh"
#include <cmath>
#include <fmt/format.h>
#include <pos_score.hh>
#include <utility>

namespace c20::search {

	int find_fifth_idx(int fourth_idx, int idx_diff)
	{
		switch (fourth_idx) {
			case 0:
				if (idx_diff == -1) return 4;
				else return 1;
			case 3:
				if (idx_diff == 1) return 7;
				else return 2;
			case 12:
				if (idx_diff == -1) return 8;
				else return 13;
			case 15:
				if (idx_diff == 1) return 11;
				else return 14;
		}
		throw std::runtime_error(fmt::format(
					"should NOT happen: corner is {}, dir is {}",
					fourth_idx, idx_diff));
	}

	bool is_corner(int idx) 
	{
		return idx == 0 || idx == 3 || 
				idx == 12 || idx == 15;
	}


	bool are_neighbours(int first_idx, int second_idx)
	{
		switch (first_idx) {
			case 0:
				return second_idx == 1 || second_idx == 4;
			case 3:
				return second_idx == 2 || second_idx == 7;
			case 12:
				return second_idx == 13 || second_idx == 8;
			case 15:
				return second_idx == 14 || second_idx == 11;
			default:
				throw std::runtime_error(fmt::format(
							"should NOT happen: corner is {}",
							first_idx));
		}
	}


	path_attr StaticPositionEval::find_main_path(const Position& pos) 
	{
		path_attr main_path{0, 0, 1};
		auto highests = pos.highest(5);
		auto first = highests[0];
		if (is_corner(first.idx)) {
			main_path.len++;
			auto second = highests[1];
			if (are_neighbours(first.idx, second.idx)) {
				//two highest next to each other
				main_path.len++;
				auto idx_diff = second.idx - first.idx;
				auto third = highests[2];
				auto third_idx = second.idx + idx_diff;
				if (third.num == pos[third_idx]) {
					//three highest next to each other
					main_path.len++;
					auto fourth = highests[3];
					auto fourth_idx = second.idx + 2* idx_diff;
					if (fourth.num == pos[fourth_idx]) {
						//four highest next to each other
						main_path.len++;
						auto fifth_idx = find_fifth_idx(fourth_idx, idx_diff);
						auto fifth = highests[4];
						if (fifth.num == pos[fifth_idx]) {
							//fifth highest next to each other
							main_path.len++;
						} else { //four in path
							main_path.breaker_num = pos[fifth_idx];
							main_path.optimal_num = fifth.num;
						}
					} else { //three in path
						main_path.breaker_num = pos[fourth_idx];
						main_path.optimal_num = fourth.num;
					}
				} else { //two in path
					main_path.breaker_num = pos[third_idx];
					main_path.optimal_num = third.num;
				}
			} else {
				//TODO
			}
		}
		return main_path;
	}

	Value StaticPositionEval::evaluate(const Position& pos)
	{
		//score multiplier based on length of high path
		Value score = 1;
		path_attr main_path = find_main_path(pos);
		switch (main_path.len) {
			case 0:
				score = 1;
				break;
			case 1:
				score = 2;
				break;
			case 2:
				score = 4;
				break;
			case 3:
				score = 8;
				break;
			case 4:
				score = 26;
				break;
			case 5:
				score = 40;
				break;
			default:
				break;
		}
		score *= 1 + 2.7 * std::pow(
				(double) (main_path.breaker_num) / main_path.optimal_num, 2);
		//TODO incentivize joining numbers
		return score;
	}

	void StaticPositionEval::batch_evaluate(std::vector<UserNode *> &, TimeSpan)
	{
		throw "not implemented";
	}
}
