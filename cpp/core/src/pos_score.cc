#include "types.hh"
#include <cmath>
#include <pos_score.hh>

namespace c20::search {

#define MMMULT 2.9
#define ELSE_MULT(idx, denom) score *= 1 + (double)(2.5*pos[idx]) *(MMMULT-2) / denom


	Value StaticPositionEval::evaluate(const Position& pos)
	{
		//score multiplier based on length of high path
		int main_path_len = 0;
		std::pair<int,int> main_path_breaker;
		auto highests = pos.highest(5);
		auto first = highests[0];
		if (
				first.idx == 0 || first.idx == 3 || 
				first.idx == 12 || first.idx == 15
		   ) {
			main_path_len++;
			auto second = highests[1];
			auto idx_diff = first.idx - second.idx;
			auto idx_diff_abs = std::abs(idx_diff);
			if (idx_diff_abs == 1 || idx_diff_abs == TABLE_SIZE) {
				main_path_len++;
				auto third = highests[2];
				auto idx_diff_2 = second.idx - third.idx;
				if (idx_diff == idx_diff_2) {
					main_path_len++;
					auto fourth = highests[3];
					auto idx_diff_3 = third.idx - fourth.idx;
					if (idx_diff == idx_diff_3) {
						main_path_len++;
						auto fifth = highests[4];
						auto idx_diff_4 = fourth.idx - fifth.idx;
						auto idx_diff_4_abs = std::abs(idx_diff_4);
						if (idx_diff_4_abs == 1 || idx_diff_4_abs == 4) {
							main_path_len++;
						}
						/* else ELSE_MULT(idx, denom); */
						//TODO add else her
					} else {
						main_path_breaker = std::pair(
								pos[third.idx + idx_diff], fourth.num);
					}
				} else {
					main_path_breaker = std::pair(
							pos[second.idx + idx_diff], third.num);
				}
			} else {
				//TODO
			}
		}
		Value score = 1;
		switch (main_path_len) {
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
				score = 16;
				break;
			case 5:
				score = 40;
				break;
			default:
				break;
		}
		score *= 1 + 5.7 * std::pow(
				(double) (main_path_breaker.first) / main_path_breaker.second, 2);
		//punish unjoined numbers
		if (main_path_len == 1) {
			/* if (highests[0].num - highests[1].num > 3) score *= 2.2*MMMULT; */
		} else if (1 < main_path_len && main_path_len < 5){
			if (highests[main_path_len-1].num == highests[main_path_len-2].num
					&& highests[main_path_len-2].num - highests[main_path_len-3].num > 3) {
				/* score *= std::pow(MMMULT, 5 - main_path_len-1); */
			}
		}
		return score;
	}

	void StaticPositionEval::batch_evaluate(std::vector<UserNode *> &, TimeSpan)
	{
		throw "not implemented";
	}
}
