import os
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'

import argparse
import csv
import logging
import subprocess
from typing import Tuple, List
import uuid

import math

import keras
import numpy as np
import pandas as pd
import tensorflow as tf
from keras import layers

logging.basicConfig(format='%(asctime)s.%(msecs)03d [%(levelname)s] %(message)s', 
        datefmt='%Y-%m-%d %H:%M:%S',
        level=logging.INFO)

def get_parser() -> argparse.ArgumentParser:
    default_cnn_dir = "./cnn/"
    default_train_data_dir= default_cnn_dir + 'train_data/'

    parser = argparse.ArgumentParser(
            formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    subparsers = parser.add_subparsers()

    csv_formatter_parser = subparsers.add_parser("csv-format")
    csv_formatter_parser.add_argument("in_file", type=str)
    csv_formatter_parser.add_argument("out_file", type=str)
    csv_formatter_parser.add_argument('--version', type=str, default='vv1')
    csv_formatter_parser.set_defaults(func=transform_game_log_entrypoint)

    new_model_parser = subparsers.add_parser("new-model")
    new_model_parser.add_argument('--model-path', type=str, 
            default=default_cnn_dir + "models/v1")
    new_model_parser.add_argument('--input-csv-path', type=str, 
            default=default_train_data_dir + "train_it_0.csv")
    new_model_parser.add_argument('--model-version', type=str, 
            default='mv1')
    new_model_parser.set_defaults(func=new_model_from_csv_entrypoint)

    train_model_parser = subparsers.add_parser('train-model')
    train_model_parser.add_argument('--input-model-path', type=str, 
            default=default_cnn_dir + "models/v1")
    train_model_parser.add_argument('--output-model-path', type=str, 
            default=default_cnn_dir + "models/v2")
    train_model_parser.add_argument('--input-csv-path', type=str, 
            default=default_train_data_dir + "train_it_1.csv")
    train_model_parser.set_defaults(func=train_model_entrypoint)

    play_games_parser = subparsers.add_parser('play-games')
    play_games_parser.add_argument('--random', action='store_true')
    play_games_parser.add_argument('--num-games', type=str, default='100')
    play_games_parser.add_argument('--out-path', type=str, 
            default=default_train_data_dir + 'random_games.csv')
    play_games_parser.add_argument('--ctwenty_bin', type=str,
            default='Release/headless_app/ctwenty48')
    play_games_parser.add_argument('--model-path', type=str)
    play_games_parser.set_defaults(func=run_games_entrypoint)

    iterate_learn_parser = subparsers.add_parser('iterate-learn', 
            help='Play games with a given model, train model on output' + 
                 'then repeat with new model.')
    iterate_learn_parser.add_argument('start_model', type=str)
    # iterate_learn_parser.add_argument('output-folder', type=str)
    iterate_learn_parser.add_argument('--num-games', type=int,
            default=20, help='Num games played before each iterations')
    iterate_learn_parser.add_argument('--num-iterations', type=int,
            default=10)
    iterate_learn_parser.set_defaults(func=iterate_learn_entrypoint)

    return parser


def play_games(ctwenty_bin: str, random: bool, num_games: str, 
        log_path: str, model_path: str):
    """
    Run games function of ctwenty48 bin, and saving the
    game logs to the specified log_path.
    """
    logging.info(('Running {} games in child process, ' +
        'saving game logs to {}').format(num_games, log_path))

    if model_path is None:
        command = [ctwenty_bin, 
            '--num', num_games, 
            '--log-path', log_path]
    else:
        command = [ctwenty_bin, model_path,
            '--num', num_games, 
            '--log-path', log_path]
    if random:
        logging.info("Random games turned on.")
        command.append('--random')

    logging.info("Running process: {}".format(command))
    process = subprocess.Popen(command)
    exit_code = process.wait()
    if (exit_code):
        logging.error('Nonzero exit code while running random games: {}'
                .format(exit_code))
        raise Exception('Failed running games')
    logging.info('Running games is done.')


def create_model(version="mv1") -> keras.Model:
    if version == 'mv1':
        input = keras.Input(shape=(16,))
        internal_1 = layers.Dense(64, activation=tf.nn.relu)(input)
        internal_2 = layers.Dense(48, activation=tf.nn.relu)(internal_1)
        output = layers.Dense(1, activation=tf.nn.sigmoid)(internal_2)
        
        model = keras.Model(inputs=input, outputs=output)
        model.compile(optimizer=keras.optimizers.SGD(),
                      loss=keras.losses.MeanSquaredError())
        logging.info('Model compiled.')
        return model
    elif version == 'mv2':
        input = keras.Input(shape=(16,))
        x = layers.Dense(64, activation=tf.nn.relu)(input)
        x = layers.Dropout(0.1)(x)
        x = layers.Dense(128, activation=tf.nn.relu)(x)
        x = layers.Dropout(0.1)(x)
        x = layers.Dense(96, activation=tf.nn.relu)(x)
        output = layers.Dense(1, activation=tf.nn.sigmoid)(x)
        
        model = keras.Model(inputs=input, outputs=output)
        model.compile(optimizer=keras.optimizers.SGD(),
                      loss=keras.losses.MeanSquaredError())
                      # loss=keras.losses.BinaryCrossentropy())
        logging.info('Model compiled.')
        return model
    raise Exception("Unknown version: " + str(version))


def train_model(model: keras.Model, x: np.array, y: np.array, epochs=10):
    input_size = y.shape[0]
    test_size = input_size // 6

    train_x = x[:-test_size]
    test_x = x[-test_size:]
    train_y = y[:-test_size]
    test_y = y[-test_size:]
    logging.info('Start fitting model.')
    model.fit(x=train_x, y=train_y,
              batch_size=512,
              epochs=epochs,
              shuffle=True,
              validation_data=(test_x, test_y))
    logging.info('Model fitted.')


def save_model(model: keras.Model, model_path: str):
    model.save(model_path, save_format='tf')
    logging.info('Model saved.')


def np_array_from_csvs(input_csv_paths: List[str]) -> Tuple[np.array, np.array]:
    x_list, y_list = [], []
    for path in input_csv_paths:
        pd_array = pd.read_csv(path)
        x = pd_array.iloc[:, :-1]
        y = pd_array.iloc[:, -1:]
        x_list.append(np.array(x))
        y_list.append(np.array(y))
    x, y = np.concatenate(x_list), np.concatenate(y_list)
    logging.info('input dims: [{}, {}]'.format(x.shape, y.shape))
    return x, y


def new_model_from_csvs(model_path: str, input_csv_paths: List[str], model_version: str,
        epochs=10):
    """
    Creates new model from scratch, and trains it on the data
    from the passed CSV file.
    """
    logging.info('create new model from {} at {}'
            .format(input_csv_paths, model_path))
    x, y = np_array_from_csvs(input_csv_paths)
    model = create_model(model_version)
    train_model(model, np.array(x), np.array(y), epochs=epochs)
    save_model(model, model_path)


def new_model_from_csv(model_path: str, input_csv_path: str, model_version: str,
        epochs=10):
    new_model_from_csvs(model_path, (input_csv_path, ), model_version, epochs)


def train_existing_model_from_csv(input_model_path: str, output_model_path: str,
        input_csv_path: str):
    logging.info('Train model at {} from {}, and place new one at {}'
            .format(input_model_path, input_csv_path, output_model_path))

    model = keras.models.load_model(input_model_path)
    x, y = np_array_from_csvs(tuple(input_csv_path))
    train_model(model, np.array(x), np.array(y))
    save_model(model, output_model_path)




#transformers

def transform_game_log(game_log_path: str, dataset_path: str,
        version: str):
    """
    Transforms log produced for a played game, into a format
    that ML-libraries can train CNN on.
    """
    logging.info(('Transforming game logs from {}, and placing it to {}' +
        "using version {}").format(game_log_path, dataset_path, version))
    if version == 'vv1':
        num_rows = transform_vv1(game_log_path, dataset_path)
    elif version == 'vv2':
        num_rows = transform_vv2(game_log_path, dataset_path)
    elif version == 'vv3':
        num_rows = transform_vv3(game_log_path, dataset_path)
    else:
        raise Exception("Unknown version: " + version)
    logging.info("Wrote {} transformed lines".format(num_rows))


def transform_vv1(game_log_path: str, dataset_path: str):
    out_rows = []
    with open(game_log_path) as csv_file:
        csv_reader = csv.reader(csv_file)
        for row in csv_reader:
            if int(row[-1]) < 100:  # game length was less then 100
                continue
            if int(row[-2]) >= 100:  # game has more than 100 moves left
                continue
            out_row = []
            out_row.extend(row[:16])
            out_row.append(int(row[-2]) / 100)
            out_rows.append(out_row)
    with open(dataset_path, "w") as out_csv_file:
        csv_writer = csv.writer(out_csv_file)
        for out_row in out_rows:
            csv_writer.writerow(out_row)
    return len(out_rows)


def transform_vv2(game_log_path: str, dataset_path: str):
    out_rows = []
    cutoff = 100
    with open(game_log_path) as csv_file:
        csv_reader = csv.reader(csv_file)
        for row in csv_reader:
            out_row = []
            out_row.extend(row[:16])
            out_row.append(min(cutoff, int(row[-2])) / cutoff)
            out_rows.append(out_row)
    with open(dataset_path, "w") as out_csv_file:
        csv_writer = csv.writer(out_csv_file)
        for out_row in out_rows:
            csv_writer.writerow(out_row)
    return len(out_rows)
  

def transform_vv3(game_log_path: str, dataset_path: str):
    in_rows = []
    max_game_length = -1
    with open(game_log_path) as csv_file:
        csv_reader = csv.reader(csv_file)
        for row in csv_reader:
            in_row = []
            for row_elem in row:
                in_row.append(int(row_elem))
            max_game_length = max(max_game_length, in_row[-1])
            in_rows.append(in_row)
            
    log_max = math.log(max_game_length)
    linear_cutoff = 200
    cutoff_multiplier = math.log(linear_cutoff) / log_max / linear_cutoff

    out_row_count = 0
    with open(dataset_path, "w") as out_csv_file:
        csv_writer = csv.writer(out_csv_file)
        for in_row in in_rows:
            out_row = [_ for _ in in_row[:16]]
            remaining_moves = in_row[-2]

            if remaining_moves > linear_cutoff:
                out_row.append(math.log(remaining_moves) / log_max)
            else:
                out_row.append(remaining_moves * cutoff_multiplier)

            csv_writer.writerow(out_row)
            out_row_count += 1
    return out_row_count


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
        new_model_from_csvs(new_model_path, dataset_path_list, 'mv2', 50)

        old_model_path = new_model_path


def print_basic_stats(log_path: str, num_games: int, iteration: int):
    with open(log_path, 'r') as fp:
        num_lines = sum(1 for line in fp)
    logging.info('Iteration no. {} averaged {} moves.'.format(iteration, 
            num_lines / num_games))

#entrypoints

def transform_game_log_entrypoint(args):
    transform_game_log(args.in_file, args.out_file, args.version)



def new_model_from_csv_entrypoint(args):
    new_model_from_csv(args.model_path, args.input_csv_path, args.model_version)


def run_games_entrypoint(args):
    play_games(args.ctwenty_bin, args.random, args.num_games,
            args.out_path, args.model_path)


def train_model_entrypoint(args):
    train_existing_model_from_csv(args.input_model_path, args.output_model_path,
            args.input_csv_path)

def iterate_learn_entrypoint(args):
    iterate_learn(args.start_model, args.num_games, args.num_iterations)


def main():
    args = get_parser().parse_args()
    args.func(args)


if __name__ == '__main__':
    main()

