import os
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'
import tensorflow as tf
import keras
from keras import layers
import numpy as np
import pandas as pd

import csv
import argparse
import logging
import subprocess



logging.basicConfig(format='%(asctime)s.%(msecs)03d [%(levelname)s] %(message)s', 
        datefmt='%Y-%m-%d %H:%M:%S',
        level=logging.INFO)

def get_parser() -> argparse.ArgumentParser:
    default_cnn_dir = "./cnn/"
    default_train_data_dir= default_cnn_dir + 'train_data/'

    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers()

    csv_formatter_parser = subparsers.add_parser("csv-format")
    csv_formatter_parser.add_argument("in_file", type=str)
    csv_formatter_parser.add_argument("out_file", type=str)
    csv_formatter_parser.set_defaults(func=transform_game_log_entrypoint)

    new_model_parser = subparsers.add_parser("new-model")
    new_model_parser.add_argument('--model-path', type=str, 
            default=default_cnn_dir + "models/v1")
    new_model_parser.add_argument('--input-csv-path', type=str, 
            default=default_train_data_dir + "/train_it_0.csv")
    new_model_parser.set_defaults(func=new_model_from_csv_entrypoint)

    play_random_games_parser = subparsers.add_parser('random-games')
    play_random_games_parser.add_argument('--num-games', type=str, default='100')
    play_random_games_parser.add_argument('--out-path', type=str, 
            default=default_train_data_dir + 'random_games.csv')
    play_random_games_parser.add_argument('--ctwenty_bin', type=str,
            default='build/bin/ctwenty48')
    play_random_games_parser.set_defaults(func=run_random_games_entrypoint)
    return parser


def play_random_games(ctwenty_bin: str, num_games: str, log_path: str):
    """
    Run the random games function of ctwenty48 bin, and saving the
    game logs to the specified log_path.
    """
    logging.info(('Running {} random games in child process, ' +
        'saving game logs to {}').format(num_games, log_path))
    process = subprocess.Popen([ctwenty_bin, "--random", 
        '--num', num_games, 
        '--log-path', log_path])
    exit_code = process.wait(timeout=10)
    if (exit_code):
        logging.error('Nonzero exit code while running random games: {}'
                .format(exit_code))
        raise Exception('Failed running random_games')
    logging.info('Running random games is done.')



def create_and_fit_new_model(model_path: str, x: np.array, y: np.array):
    input = keras.Input(shape=(16,))
    internal_1 = layers.Dense(64, activation=tf.nn.relu)(input)
    internal_2 = layers.Dense(48, activation=tf.nn.relu)(internal_1)
    output = layers.Dense(1, activation=tf.nn.sigmoid)(internal_2)
    
    model = keras.Model(inputs=input, outputs=output)
    model.compile(optimizer=keras.optimizers.SGD(),
                  loss=keras.losses.MeanSquaredError())
    logging.info('Model compiled.')

    input_size = y.shape[0]
    test_size = input_size // 6

    train_x = x[:-test_size]
    test_x = x[-test_size:]
    train_y = y[:-test_size]
    test_y = y[-test_size:]
    logging.info('Start fitting model.')
    model.fit(x=train_x, y=train_y,
              batch_size=64,
              epochs=100,
              validation_data=(test_x, test_y))
    logging.info('Model fitted.')

    model.save(model_path, save_format='tf')
    logging.info('Model saved.')


def new_model_from_csv(model_path: str, input_csv_path: str):
    """
    Creates new model from scratch, and trains it on the data
    from the passed CSV file.
    """
    logging.info('create new model from {} at {}'
            .format(input_csv_path, model_path))
    input_vecs = pd.read_csv(input_csv_path)
    x = input_vecs.iloc[:, :-1]
    y = input_vecs.iloc[:, -1:]
    logging.info('input dims: [{}, {}]'.format(x.shape, y.shape))
    create_and_fit_new_model(model_path, np.array(x), np.array(y))


def transform_game_log(game_log_path: str, dataset_path: str):
    """
    Transforms log produced for a played game, into a format
    that ML-libraries can train CNN on.
    """
    logging.info('Transforming game logs from {}, and placing it to {}'
            .format(game_log_path, dataset_path))
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
    logging.info("Wrote {} transformed lines".format(len(out_rows)))



#entrypoints

def transform_game_log_entrypoint(args):
    transform_game_log(args.in_file, args.out_file)



def new_model_from_csv_entrypoint(args):
    new_model_from_csv(args.model_path, args.input_csv_path)


def run_random_games_entrypoint(args):
    play_random_games(args.ctwenty_bin, args.num_games, args.out_path)


def main():
    args = get_parser().parse_args()
    args.func(args)


if __name__ == '__main__':
    main()

