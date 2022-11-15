import csv
import logging
import math

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
    elif version == 'vv3':
        num_rows = transform_vv3(game_log_path, dataset_path)
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
    out_rows = []
    cutoff = 100
    with open(game_log_path) as csv_file:
        csv_reader = csv.reader(csv_file)
        for row in csv_reader:
            out_row = []
            out_row.extend(row[:16])
            out_row.append(min(cutoff, int(row[-2])) / cutoff)
            out_rows.append(out_row)
    with open(dataset_path, "w") as out_csv_file:
        csv_writer = csv.writer(out_csv_file)
        for out_row in out_rows:
            csv_writer.writerow(out_row)
    return len(out_rows)
  

def transform_vv3(game_log_path: str, dataset_path: str):
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
            
    log_max = math.log(max_game_length)
    linear_cutoff = 200
    cutoff_multiplier = math.log(linear_cutoff) / log_max / linear_cutoff

    out_row_count = 0
    with open(dataset_path, "w") as out_csv_file:
        csv_writer = csv.writer(out_csv_file)
        for in_row in in_rows:
            out_row = [_ for _ in in_row[:16]]
            remaining_moves = in_row[-2]

            if remaining_moves > linear_cutoff:
                out_row.append(math.log(remaining_moves) / log_max)
            else:
                out_row.append(remaining_moves * cutoff_multiplier)

            csv_writer.writerow(out_row)
            out_row_count += 1
    return out_row_count


