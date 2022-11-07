#! /bin/bash
rm -rf neural-net/models/v3
# python3 neural-net/tools/train_management.py play-games --num-games 50 --out-path cnn/train_data/games_it_1.csv
python3 neural-net/tools/train_management.py csv-format cnn/train_data/games_it_1.csv cnn/train_data/train_it_1.csv --version vv3
python3 neural-net/tools/train_management.py new-model --model-path cnn/models/v3 --input-csv-path cnn/train_data/train_it_1.csv --model-version mv2

