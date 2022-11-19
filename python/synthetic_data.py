"""
This file contains scripts that generates datasets for training
from randomly generated data, or synthetic data.

Either using arbitrary scoring (fully synthetic).
Or by playing games from given positions (to be implemented).
"""

from typing import Tuple, List
import random
import numpy as np


def create_synthetic_data() -> Tuple[List[int], List[int]]:
    positions = []
    evals = []
    
    reps = 100
    #from best to worst
    #1path
    partition = Partition([main_path(0,8)])
    for _ in range(reps):
        pos = (ValuedPartition(partition)
                .fill_path_desc_random_no_zeros(12, 3)
                .fill_rest_random(3, 3).position)
        add_symmetries(pos, 1.0, positions, evals)

    partition = Partition([main_path(0,5)])
    for _ in range(reps):
        pos = (ValuedPartition(partition)
                .fill_path_desc_random_no_zeros(11, 3)
                .fill_rest_random(3, 3).position)
        add_symmetries(pos, 0.975, positions, evals)

    partition = Partition([main_path(0,3)])
    for _ in range(reps):
        pos = (ValuedPartition(partition)
                .fill_path_desc_random_no_zeros(10, 4)
                .fill_rest_random(1, 3).position)
        add_symmetries(pos, 0.95, positions, evals)

    partition = Partition([main_path(0,2)])
    for _ in range(reps):
        pos = (ValuedPartition(partition)
                .fill_path_desc_random_no_zeros(10, 6)
                .fill_rest_random(5, 4).position)
        add_symmetries(pos, 0.925, positions, evals)

    partition = Partition([main_path(0,2)])
    for _ in range(reps):
        pos = (ValuedPartition(partition)
                .fill_path_desc_random_no_zeros(11, 7)
                .fill_rest_random(8, 5).position)
        add_symmetries(pos, 0.925, positions, evals)

    #2paths
    partition = Partition([main_path(0,3), main_path(3,4)])
    for _ in range(reps):
        pos = (ValuedPartition(partition)
                .fill_path_desc_random_no_zeros(11, 6)
                .fill_path_desc_random_no_zeros(9, 4)
                .fill_rest_random(2, 3).position)
        add_symmetries(pos, 0.9, positions, evals)

    partition = Partition([main_path(0,2), main_path(2,4)])
    for _ in range(reps):
        pos = (ValuedPartition(partition)
                .fill_path_desc_random_no_zeros(9, 6)
                .fill_path_desc_random_no_zeros(8, 4)
                .fill_rest_random(3, 2).position)
        add_symmetries(pos, 0.85, positions, evals)

    partition = Partition([main_path(0,3), main_path(3,2)])
    for _ in range(reps):
        pos = (ValuedPartition(partition)
                .fill_path_desc_random_no_zeros(12, 4)
                .fill_path_desc_random_no_zeros(9, 4)
                .fill_rest_random(4, 3).position)
        add_symmetries(pos, 0.8, positions, evals)

    #1 sqare and 1 path / paths
    partition = Partition([Path([0]), main_path(1,4)])
    for _ in range(reps):
        pos = (ValuedPartition(partition)
                .fill_path_exact([9])
                .fill_path_desc_random_no_zeros(10, 4)
                .fill_rest_random(4, 3).position)
        add_symmetries(pos, 0.8, positions, evals)

    partition = Partition([Path([0]), main_path(1,4)])
    for _ in range(reps):
        pos = (ValuedPartition(partition)
                .fill_path_exact([8])
                .fill_path_desc_random_no_zeros(10, 4)
                .fill_rest_random(4, 3).position)
        add_symmetries(pos, 0.7, positions, evals)

    partition = Partition([Path([0]), main_path(1,4)])
    for _ in range(reps):
        pos = (ValuedPartition(partition)
                .fill_path_exact([6])
                .fill_path_desc_random_no_zeros(9, 4)
                .fill_rest_random(4, 3).position)
        add_symmetries(pos, 0.6, positions, evals)

    partition = Partition([Path([0]), main_path(1,4)])
    for _ in range(reps):
        pos = (ValuedPartition(partition)
                .fill_path_exact([5])
                .fill_path_desc_random_no_zeros(10, 5)
                .fill_rest_random(4, 3).position)
        add_symmetries(pos, 0.5, positions, evals)

    partition = Partition([Path([0]), main_path(1,4)])
    for _ in range(reps):
        pos = (ValuedPartition(partition)
                .fill_path_exact([4])
                .fill_path_desc_random_no_zeros(10, 5)
                .fill_rest_random(4, 3).position)
        add_symmetries(pos, 0.4, positions, evals)

    #2paths
    partition = Partition([main_path(0,2), main_path(2,4)])
    for _ in range(reps):
        pos = (ValuedPartition(partition)
                .fill_path_desc_random_no_zeros(8,7)
                .fill_path_desc_random_no_zeros(10, 4)
                .fill_rest_random(4, 3).position)
        add_symmetries(pos, 0.4, positions, evals)
    
    partition = Partition([main_path(0,2), main_path(2,4)])
    for _ in range(reps):
        pos = (ValuedPartition(partition)
                .fill_path_desc_random_no_zeros(8,6)
                .fill_path_desc_random_no_zeros(10, 4)
                .fill_rest_random(4, 3).position)
        add_symmetries(pos, 0.3, positions, evals)

    partition = Partition([main_path(0,2), main_path(2,4)])
    for _ in range(reps):
        pos = (ValuedPartition(partition)
                .fill_path_desc_random_no_zeros(7,5)
                .fill_path_desc_random_no_zeros(11, 4)
                .fill_rest_random(1, 2).position)
        add_symmetries(pos, 0.3, positions, evals)

    partition = Partition([main_path(0,2), main_path(2,4)])
    for _ in range(reps):
        pos = (ValuedPartition(partition)
                .fill_path_desc_random_no_zeros(5,2)
                .fill_path_desc_random_no_zeros(10, 4)
                .fill_rest_random(4, 3).position)
        add_symmetries(pos, 0.25, positions, evals)

    #custom paths
    partition = Partition([Path([0,1,5,9,10]), main_path(2,4)])
    for _ in range(reps):
        pos = (ValuedPartition(partition)
                .fill_path_exact([2, 7, 6, 5, 4])
                .fill_path_desc_random_no_zeros(10, 4)
                .fill_rest_random(4, 3).position)
        add_symmetries(pos, 0.35, positions, evals)

    partition = Partition([Path([0,1,5,9,10]), main_path(2,4)])
    for _ in range(reps):
        pos = (ValuedPartition(partition)
                .fill_path_exact([2, 6, 5, 4, 2])
                .fill_path_desc_random_no_zeros(10, 4)
                .fill_rest_random(4, 3).position)
        add_symmetries(pos, 0.3, positions, evals)

    partition = Partition([Path([0,1,5,9,10]), main_path(2,4)])
    for _ in range(reps):
        pos = (ValuedPartition(partition)
                .fill_path_exact([2, 5, 4, 2, 1])
                .fill_path_desc_random_no_zeros(10, 4)
                .fill_rest_random(4, 3).position)
        add_symmetries(pos, 0.2, positions, evals)

    #random shuffle
    partition = Partition([])
    for i in range(reps *10): 
        pos = (ValuedPartition(partition)
                .fill_rest_random(i % 4, i % 8 + 2).position)
        add_symmetries(pos, 0.0, positions, evals)
    return positions, evals

#util classes and functions

class Path:
    def __init__(self, indices: List[int]):
        self.indices = tuple(indices)


class Partition:
    def __init__(self, paths: List[Path]):
        self.paths = paths
        rest = {i for i in range(16)}
        for path in paths:
            for idx in path.indices:
                rest.remove(idx)
        self.rest = tuple(rest)



class ValuedPartition:
    def __init__(self, part: Partition):
        self.partition = part
        self.position = [0] *16
        self.__current_idx = 0

    def fill_path_desc_random_no_zeros(self, high: int, low: int):
        idx = self.__current_idx
        path = self.partition.paths[idx]

        random_bits = random_set_bits(
                len(path.indices) + high-low -2, high-low)
        current_val = high
        self.position[path.indices[0]] = high
        path_idx = 1
        for bit_set in random_bits:
            if bit_set:
                current_val -= 1
            else:
                self.position[path.indices[path_idx]] = current_val
                path_idx += 1
        self.position[path.indices[path_idx]] = low

        self.__current_idx += 1
        return self

    #TODO fill strict desc

    def fill_path_exact(self, values: List[int]):
        idx = self.__current_idx
        path = self.partition.paths[idx]

        for idx, val in zip(path.indices, values):
            self.position[idx] = val

        self.__current_idx += 1
        return self


    def fill_rest_random(self, num_zeros: int, high: int):
        elements = np.array(tuple(i for i in range(1, high +1)))
        prob_sum = sum(tuple(1/i for i in range(1, high +1)))
        weights = np.array(tuple(1/i/prob_sum for i in range(1, high +1)))

        rest_size = len(self.partition.rest)
        filled = 0
        while filled < rest_size - num_zeros:
            rand_rest_idx = random.randint(0, rest_size -1)
            pos_idx = self.partition.rest[rand_rest_idx]
            if self.position[pos_idx] == 0:
                rand_elem = np.random.choice(elements, p=weights)
                self.position[pos_idx] = rand_elem
                filled += 1

        return self




#util functions for creating paths, partitions

def main_path(start_idx: int, length: int) -> Path:
    return Path(tuple(sorted_indices[i] 
        for i in range(start_idx, start_idx + length)))


def random_set_bits(num_bits: int, num_set: int) -> List[int]:
    bits = [0] * num_bits
    set_bits = 0
    while set_bits < num_set:
        rand_idx = random.randint(0, num_bits -1)
        if bits[rand_idx] == 0:
            set_bits += 1
            bits[rand_idx] = 1
    return bits


# dubious
# def bin_count(numbers: List[int]) -> OrderedDict:
#     bins = OrderedDict()
#     for num in numbers:
#         if num in bins:
#             bins[num] += 1
#         else:
#             bins[num] = 1
#     return bins


# def random_bins():
#     bins = [0] * 16
#     elements = np.array((i for i in range(16)))
#     weights = np.array((1/i for i in range(16)))
#     for _ in range(16):
#         random_val = np.random.choice(elements, weights)
#         bins[random_val] += 1
#     return bins


def symmetries(pos: List[int]) -> List[List[int]]:
    syms = [pos, mirror_sym(pos)]
    for _ in range(3):
        pos = rotate_sym(pos)
        syms.extend([pos, mirror_sym(pos)])
    return syms
    

def mirror_sym(numbers: List[int]) -> List[int]:
    mirrored = []
    for i in range(4):
        for j in range(4):
            mirrored.append(numbers[i*4 + 3-j])
    return mirrored

def rotate_sym(numbers: List[int]) -> List[int]:
    rotated = []
    for i in range(4):
        for j in range(4):
            rotated.append(numbers[i + (3-j) *4])
    return rotated

def add_symmetries(pos: List[int], pos_eval: float, pos_list, eval_list):
    for pos_sym in symmetries(pos):
        pos_list.append(pos_sym)
        eval_list.append(pos_eval)

sorted_indices = [
        0,1,2,3,
        7,6,5,4,
        8,9,10,11,
        15,14,13,12
        ]




# testing

def print_pos(pos: List[int]):
    for i in range(4):
        print(pos[i*4: (i+1)*4])
    print('_____')

if __name__ == '__main__':
    partition = Partition((main_path(0, 5), main_path(5, 4)))

    print_pos(ValuedPartition(partition)
            .fill_path_desc_random_no_zeros(10,4)
            .fill_path_desc_random_no_zeros(7,4)
            .fill_rest_random(3, 2)
            .position)

    partition = Partition((main_path(0,3), Path([3]), main_path(4,5)))
    pos = (ValuedPartition(partition)
            .fill_path_desc_random_no_zeros(10, 6)
            .fill_path_exact([2])
            .fill_path_desc_random_no_zeros(6,2)
            .fill_rest_random(1, 3).position)
    # print_pos(pos)
    # print_pos(rotate_sym(pos))
    # print_pos(mirror_sym(pos))
    for sym_pos in symmetries(pos):
        print_pos(sym_pos)

    positions, evals = create_synthetic_data()
    # print(positions)




