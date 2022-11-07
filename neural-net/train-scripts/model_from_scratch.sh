#! /bin/bash
python3 neural-net/tools/train_management.py play-games --random
python3 neural-net/tools/train_management.py csv-format cnn/train_data/random_games.csv cnn/train_data/train_it_0.csv
python3 neural-net/tools/train_management.py new-model --model-path cnn/models/v1 --input-csv-path cnn/train_data/train_it_0.csv

