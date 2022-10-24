import os
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'

import argparse
import csv
import logging
import subprocess
from typing import Tuple

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
            default='Release/bin/ctwenty48')
    play_games_parser.add_argument('--model-path', type=str)
    play_games_parser.set_defaults(func=run_games_entrypoint)

    return parser


def play_games(ctwenty_bin: str, random: bool, num_games: str, 
        log_path: str, model_path: str):
    """
    Run games function of ctwenty48 bin, and saving the
    game logs to the specified log_path.
    """
    logging.info(('Running {} games in child process, ' +
        'saving game logs to {}').format(num_games, log_path))

    command = [ctwenty_bin, 
        '--num', num_games, 
        '--log-path', log_path]
    if random:
        logging.info("Random games turned on.")
        command.append('--random')
    if model_path is not None:
        logging.info("Setting model path to {}".format(model_path))
        command.append(model_path)

    logging.info("Running process: {}".format(command))
    process = subprocess.Popen(command)
    exit_code = process.wait()
    if (exit_code):
        logging.error('Nonzero exit code while running random games: {}'
                .format(exit_code))
        raise Exception('Failed running games')
    logging.info('Running games is done.')


def create_model() -> keras.Model:
    input = keras.Input(shape=(16,))
    internal_1 = layers.Dense(64, activation=tf.nn.relu)(input)
    internal_2 = layers.Dense(48, activation=tf.nn.relu)(internal_1)
    output = layers.Dense(1, activation=tf.nn.sigmoid)(internal_2)
    
    model = keras.Model(inputs=input, outputs=output)
    model.compile(optimizer=keras.optimizers.SGD(),
                  loss=keras.losses.MeanSquaredError())
    logging.info('Model compiled.')
    return model


def train_model(model: keras.Model, x: np.array, y: np.array):
    input_size = y.shape[0]
    test_size = input_size // 6

    train_x = x[:-test_size]
    test_x = x[-test_size:]
    train_y = y[:-test_size]
    test_y = y[-test_size:]
    logging.info('Start fitting model.')
    model.fit(x=train_x, y=train_y,
              batch_size=512,
              epochs=25,
              validation_data=(test_x, test_y))
    logging.info('Model fitted.')


def save_model(model: keras.Model, model_path: str):
    model.save(model_path, save_format='tf')
    logging.info('Model saved.')

def np_array_from_csv(input_csv_path: str) -> Tuple[np.array, np.array]:
    input_vecs = pd.read_csv(input_csv_path)
    x = input_vecs.iloc[:, :-1]
    y = input_vecs.iloc[:, -1:]
    logging.info('input dims: [{}, {}]'.format(x.shape, y.shape))
    return np.array(x), np.array(y)


def new_model_from_csv(model_path: str, input_csv_path: str):
    """
    Creates new model from scratch, and trains it on the data
    from the passed CSV file.
    """
    logging.info('create new model from {} at {}'
            .format(input_csv_path, model_path))
    x, y = np_array_from_csv(input_csv_path)
    model = create_model()
    train_model(model, np.array(x), np.array(y))
    save_model(model, model_path)


def train_existing_model_from_csv(input_model_path: str, output_model_path: str,
        input_csv_path: str):
    logging.info('Train model at {} from {}, and place new one at {}'
            .format(input_model_path, input_csv_path, output_model_path))

    model = keras.models.load_model(input_model_path)
    x, y = np_array_from_csv(input_csv_path)
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
    cutoff_start = 200
    cutoff_middle = 50
    out_row_count = 0
    with open(dataset_path, "w") as out_csv_file:
        csv_writer = csv.writer(out_csv_file)
        for in_row in in_rows:
            if in_row[-1] < cutoff_start: continue
            if in_row[-2] > cutoff_start: continue

            out_row = [_ for _ in in_row[:16]]
            base_value = in_row[-1] / max_game_length

            if in_row[-2] > cutoff_middle:
                out_row.append(base_value)    
            else:
                out_row.append(base_value * (in_row[-2] / cutoff_middle))

            csv_writer.writerow(out_row)
            out_row_count += 1
    return out_row_count


#entrypoints

def transform_game_log_entrypoint(args):
    transform_game_log(args.in_file, args.out_file, args.version)



def new_model_from_csv_entrypoint(args):
    new_model_from_csv(args.model_path, args.input_csv_path)


def run_games_entrypoint(args):
    play_games(args.ctwenty_bin, args.random, args.num_games,
            args.out_path, args.model_path)


def train_model_entrypoint(args):
    train_existing_model_from_csv(args.input_model_path, args.output_model_path,
            args.input_csv_path)


def main():
    args = get_parser().parse_args()
    args.func(args)


if __name__ == '__main__':
    main()

