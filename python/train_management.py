import logging
import os

import argparse
from typing import Tuple, List
import uuid


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

#entrypoints

def transform_game_log_entrypoint(args):
    from log_parser import transform_game_log
    transform_game_log(args.in_file, args.out_file, args.version)



def new_model_from_csv_entrypoint(args):
    import models
    models.new_model_from_csv(
            args.model_path, args.input_csv_path, args.model_version)


def run_games_entrypoint(args):
    from game_play import play_games
    play_games(args.ctwenty_bin, args.random, args.num_games,
            args.out_path, args.model_path)


def train_model_entrypoint(args):
    import models
    models.train_existing_model_from_csv(
            args.input_model_path, 
            args.output_model_path,
            args.input_csv_path)


def iterate_learn_entrypoint(args):
    from play_n_learn import iterate_learn
    iterate_learn(args.start_model, args.num_games, args.num_iterations)


def main():
    args = get_parser().parse_args()
    args.func(args)


if __name__ == '__main__':
    main()

