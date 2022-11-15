import logging
import subprocess

def play_games(ctwenty_bin: str, random: bool, num_games: str, 
        log_path: str, model_path: str):
    """
    Run games function of ctwenty48 bin, and saving the
    game logs to the specified log_path.
    """
    logging.info(('Running {} games in child process, ' +
        'saving game logs to {}').format(num_games, log_path))

    if model_path is None:
        command = [ctwenty_bin, 
            '--num', num_games, 
            '--log-path', log_path]
    else:
        command = [ctwenty_bin, model_path,
            '--num', num_games, 
            '--log-path', log_path]
    if random:
        logging.info("Random games turned on.")
        command.append('--random')

    logging.info("Running process: {}".format(command))
    process = subprocess.Popen(command)
    exit_code = process.wait()
    if (exit_code):
        logging.error('Nonzero exit code while running random games: {}'
                .format(exit_code))
        raise Exception('Failed running games')
    logging.info('Running games is done.')

