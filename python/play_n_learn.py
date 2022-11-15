import os
import uuid
import logging

from game_play import play_games
from log_parser import transform_game_log
import models


def iterate_learn(start_model_path: str, num_games: int, num_iterations: int):
    binary_path = 'Release/headless_app/ctwenty48'

    iteration_uuid = str(uuid.uuid4())
    data_folder = 'cnn/train_data/' + iteration_uuid
    models_folder = 'cnn/models/' + iteration_uuid

    os.mkdir(data_folder)
    os.mkdir(models_folder)

    old_model_path = start_model_path
    dataset_path_list = []
    for it in range(num_iterations):
        # play games with model from previous iterations
        log_path = data_folder + '/game_' + str(it) + '.csv'
        play_games(binary_path, False, str(num_games), log_path, old_model_path)
        print_basic_stats(log_path, num_games, it)
        #transform game logs
        dataset_path = data_folder + '/data_' + str(it) + '.csv'
        dataset_path_list.append(dataset_path)
        transform_game_log(log_path, dataset_path, 'vv3')
        #train new model
        new_model_path= models_folder + '/' + str(it)
        models.new_model_from_csvs(new_model_path, dataset_path_list, 'mv2', 50)

        old_model_path = new_model_path


def print_basic_stats(log_path: str, num_games: int, iteration: int):
    with open(log_path, 'r') as fp:
        num_lines = sum(1 for line in fp)
    logging.info('Iteration no. {} averaged {} moves.'.format(iteration, 
            num_lines / num_games))

