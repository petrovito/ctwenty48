
import tensorflow as tf
import keras
from keras import layers
import numpy as np
import pandas as pd

import csv
import argparse


parser = argparse.ArgumentParser()
subparsers = parser.add_subparsers()

csv_formatter_parser = subparsers.add_parser("csv-format")
csv_formatter_parser.add_argument("in_file", type=str)
csv_formatter_parser.add_argument("out_file", type=str)

new_model_parser = subparsers.add_parser("new-model")
new_model_parser.add_argument('--model_path', type=str, default="./models/v1")
new_model_parser.add_argument('--input-csv-path', type=str, default="./train_log_data/random_games.csv")



def create_and_fit_new_model(model_path: str, x: np.array, y: np.array):
    input = keras.Input(shape=(16,))
    internal_1 = layers.Dense(64, activation=tf.nn.relu)(input)
    internal_2 = layers.Dense(48, activation=tf.nn.relu)(internal_1)
    output = layers.Dense(1, activation=tf.nn.sigmoid)(internal_2)
    
    model = keras.Model(inputs=input, outputs=output)
    model.compile(optimizer=keras.optimizers.SGD(),
                  loss=keras.losses.MeanSquaredError())

    input_size = len(y)
    test_size = input_size // 6

    train_x = x[:-test_size]
    test_x = x[-test_size:]
    train_y = y[:-test_size]
    test_y = y[-test_size:]
    model.fit(x=train_x, y=train_y,
              batch_size=64,
              epochs=10,
              validation_data=(test_x, test_y))

    model.save(model_path, save_format='tf')


def new_model_from_csv(model_path: str, input_csv_path: str):
    input_vecs = pd.read_csv(input_csv_path)
    x = input_vecs.iloc[:, :-1]
    y = input_vecs.iloc[:, -1:-1]
    create_and_fit_new_model(model_path, np.array(x), np.array(y))


def transform_game_log(game_log_path: str, dataset_path: str):
    """
    Transforms log produced for a played game, into a format
    that ML-libraries can train CNN on.
    """
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

#entrypoints

def transform_game_log_entrypoint(args):
    transform_game_log(args.in_file, args.out_file)

csv_formatter_parser.set_defaults(func=transform_game_log_entrypoint)


def new_model_from_csv_entrypoint(args):
    new_model_from_csv(args.model_path, args.input_csv_path)

new_model_parser.set_defaults(func=new_model_from_csv_entrypoint)


def main():
    args = parser.parse_args()
    args.func(args)


if __name__ == '__main__':
    main()

