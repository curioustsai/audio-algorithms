import os
import argparse
import subprocess
import glob

def eval(logFile, type):
    with open(logFile) as log:
        logData = log.readlines()
        for line in logData:
            if type in line.lower():
                return True
    return False


def false_alarm_eval(logFile):
    with open(logFile) as log:
        logChar = log.read(1)
        if not logChar:
            return True
        else:
            return False
        

def detect_process(inFile, logFile):
    rootDir = os.path.abspath(os.path.dirname(os.path.abspath(__file__)) + '/../')
    audioEventCmd = os.path.join(rootDir, 'build/x86/MinSizeRel/rootfs/bin/ui_audio_events')
    cmd = [audioEventCmd, '--inFile', inFile, '--logFile', logFile]
    subprocess.run(cmd)

def main():
    parser = argparse.ArgumentParser(
        description="Audio event unittest process.")
    parser.add_argument('--type', type=str,
                        help='The audio event type for test', required=True)
    parser.add_argument('--dataset', type=str,
                        help='The folder path of input files for test', required=True)
    parser.add_argument('--output', type=str,
                        help='The output file path for test', required=True)
    p = parser.parse_args()

    evalType = p.type
    dataFolder = os.path.abspath(p.dataset)
    logFolder = os.path.abspath(p.output)

    for f in glob.glob(os.path.join(dataFolder, '*.wav')):
        _, fileName = os.path.split(f)
        logFile = os.path.join(logFolder, fileName.replace('.wav', '.txt'))

        detect_process(f, logFile)

        if p.type == 'false_alarm':
            assert false_alarm_eval(logFile), "False alarm test fail on {}".format(fileName)
        else:
            assert eval(logFile, evalType), "{} test fail on {}".format(evalType.upper(), fileName)


if __name__ == "__main__":
    main()