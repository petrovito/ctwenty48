#! /bin/bash

rm -rf neural-net/models/v2
python3 neural-net/tools/train_management.py play-games --num-games 50 --out-path cnn/train_data/games_it_1.csv
python3 neural-net/tools/train_management.py csv-format cnn/train_data/games_it_1.csv cnn/train_data/train_it_1.csv --version vv3
python3 neural-net/tools/train_management.py train-model

