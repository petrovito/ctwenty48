
import csv
import argparse


parser = argparse.ArgumentParser()
parser.add_argument("in_file", type=str)
parser.add_argument("out_file", type=str)






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


def main():
    args = parser.parse_args()
    transform_game_log(args.in_file, args.out_file)


if __name__ == '__main__':
    main()

