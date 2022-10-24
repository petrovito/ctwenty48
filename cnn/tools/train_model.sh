#! /bin/bash

rm -rf cnn/models/v2
python3 cnn/tools/train_management.py play-games --num-games 100 --out-path cnn/train_data/games_it_1.csv
python3 cnn/tools/train_management.py csv-format cnn/train_data/games_it_1.csv cnn/train_data/train_it_1.csv --version vv1
python3 cnn/tools/train_management.py train-model

