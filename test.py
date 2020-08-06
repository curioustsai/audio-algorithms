from glob import glob
import os
import subprocess

BUILD_TYPE = 'Debug'

INPUT_FOLDER = './data/samples_alarm/'
INPUT_FORMAT = '*.wav'
OUTPUT_FOLDER = './data/samples_alarm_result/'

EXECUTE_BIN = './build/x86/' + BUILD_TYPE + '/rootfs/bin/ui_alarm_detection'

if not os.path.exists(OUTPUT_FOLDER):
    os.mkdir(OUTPUT_FOLDER)

INPUT_FILES = glob(os.path.join(INPUT_FOLDER, INPUT_FORMAT))

for infile in INPUT_FILES:
    basefile = os.path.basename(infile)
    # outfile = os.path.join(OUTPUT_FOLDER, basefile.replace('.wav', '-result.wav'))

    for threshold in [-15, -20, -25]:
        outfile = os.path.join(OUTPUT_FOLDER, basefile.replace('.wav', '-thr' + str(threshold) + '-result.wav'))
        cmd = EXECUTE_BIN + ' "' + infile + '" "' + outfile + '" "' + str(threshold) + '"'
        print(cmd)
        subprocess.call(cmd, shell=True)
