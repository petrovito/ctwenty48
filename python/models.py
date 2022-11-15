import os
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'

import keras
import numpy as np
import pandas as pd
import tensorflow as tf
from keras import layers

import logging
from typing import Tuple, List

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

