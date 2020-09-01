import argparse
import os
import subprocess
import numpy as np
from glob import glob
from scipy.io import wavfile
import shutil

def collect_failure(filelist, folder):
    for f in filelist:
        bname = os.path.basename(f)
        new_path = os.path.join(folder, bname)
        os.rename(f, new_path)


def check_result(outfile):
    [fs, waveform] = wavfile.read(outfile)

    if waveform.shape[1] >= 2 and np.any(waveform[:, 1] > 0):
        return False

    return True


if __name__ == "__main__":

    parser = argparse.ArgumentParser(description='Batch run the simulation')

    parser.add_argument('-b',
                        dest='build_type',
                        help='buil type: Debug, Release, MinSizeRel',
                        default='MinSizeRel',
                        type=str)

    parser.add_argument('-i',
                        dest='input_folder',
                        help='specify the input folder',
                        default='./data/samples_alarm',
                        type=str)

    parser.add_argument('-o',
                        dest='output_folder',
                        help='specify the output folder',
                        default='./data/samples_alarm_result',
                        type=str)

    parser.add_argument('--smokeThreshold',
                        dest='smokeThreshold',
                        help='smoke threshold for algorithm',
                        default='-20',
                        type=str)

    parser.add_argument('--coThreshold',
                        dest='coThreshold',
                        help='co threshold for algorithm',
                        default='-20',
                        type=str)

    parser.add_argument('-f',
                        dest='log_file',
                        help='the name of log file',
                        default='',
                        type=str)

    parser.add_argument('-d',
                        dest='target_dir',
                        help='collect failure under folder',
                        default='',
                        type=str)

    args = parser.parse_args()
    build_type = args.build_type
    input_folder = args.input_folder
    output_folder = args.output_folder
    smokeThreshold = args.smokeThreshold
    coThreshold = args.coThreshold
    log_file = args.log_file
    target_dir = args.target_dir

    # only support wav files now
    input_format = '*.wav'
    execute_bin = './build/x86/' + build_type + '/rootfs/bin/ui_audio_events'

    if not os.path.exists(output_folder):
        os.mkdir(output_folder)

    input_files = glob(os.path.join(input_folder, input_format))

    failed_list = []
    for infile in input_files:
        outfile = os.path.basename(infile).replace('.wav', '-result.wav')
        outfile = os.path.join(output_folder, outfile)
        cmd = [execute_bin, '-i \'' + infile + '\' -o \'' + outfile + '\' --smokeThreshold ', smokeThreshold, '--coThreshold ', coThreshold]
        cmd = ' '.join(cmd)
        print(cmd)
        subprocess.call(cmd, shell=True)

        result = check_result(outfile)
        if not result:
            failed_list.append(outfile)

    print('failed list:')
    print('\n'.join(failed_list))

    if log_file != '':
        with open(log_file, "w") as fd:
            fd.write('\n'.join(failed_list))

    if target_dir != '':
        if os.path.exists(target_dir):
            shutil.rmtree(target_dir)
        os.mkdir(target_dir)
        collect_failure(failed_list, target_dir)

