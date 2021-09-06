# Click Removal
# step 1: find freq > 4kHz with large engery and frame engery large enough
# step 2: lower their energy
# disccuss: how much frame should be
# this is a sample-based algorith, our frame size can depends on click sound samples
# test with 512, 1024, etc.
# lpc could be improvement

import librosa
import os, glob
import numpy as np
import matplotlib.pyplot as plt
from scipy import signal
from scipy.io import wavfile

#
# class BiquadFilter():
#     def __init__(self, ba):
#         self._coef = ba
#         self._state = np.zeros(2)
#
#     def process(self, x):
#         b0 = self._coef[0]
#         b1 = self._coef[1]
#         b2 = self._coef[2]
#         a1 = self._coef[3]
#         a2 = self._coef[4]
#
#         y = np.zeros(len(x))
#         for i in np.arange(len(x)):
#             temp1 = a1 * self._state[0]
#             temp2 = a2 * self._state[1]
#             wn = x[i] - temp1 - temp2
#             y[i] = b0 * wn + b1 * self._state[0] + b2 * self._state[1]
#
#             self._state[1] = self._state[0]
#             self._state[0] = wn
#         return y


class ClickRemover():
    def __init__(self, frameSize, threshold_all, threshold_4kHz):
        self._sos_hpf4k = signal.cheby1(4, 3, 4000/24000, 'high', output='sos')
        print(self._sos_hpf4k)
        self._frameSize = frameSize
        self._hopSize = frameSize // 2
        self._threshold_all = threshold_all
        self._threshold_4kHz = threshold_4kHz

    def process(self, inputfile, outputfile, en_plot=False):
        fs, x = wavfile.read(inputfile)
        x = x / (2**15)  # convert to float [0, 1)
        y = signal.sosfilt(self._sos_hpf4k, x)
        y_int16 = y * (2**15)
        wavfile.write("biquad_py.wav", fs, y_int16.astype(np.int16))

        frame_count = 0
        num_frames = len(x) // self._hopSize
        buf_frame_eng = np.zeros(num_frames)
        buf_frame_eng_hpf = np.zeros(num_frames)
        buf_frame_detected = np.zeros(num_frames)
        filtered_output = np.copy(x)

        start = 0
        frame_eng_prev = 0

        while frame_count < num_frames:
            end = start + self._frameSize
            data = x[start:end]
            data_hpf = y[start:end]

            frame_eng = np.sum(data ** 2) / self._frameSize
            frame_eng_hpf = np.sum(data_hpf ** 2) / self._frameSize

            buf_frame_eng[frame_count] = frame_eng
            buf_frame_eng_hpf[frame_count] = frame_eng_hpf

            detected = 0
            lpc_remain = 0

            if frame_eng > self._threshold_all and frame_eng_hpf > self._threshold_4kHz:
                detected = 1
                buf_frame_detected[frame_count] = 1

                gain = frame_eng_prev / frame_eng
                filtered_output[start:end] *= gain

            alpha = 0.2
            frame_eng_prev = (1 - alpha) * frame_eng_prev + alpha * frame_eng
            start += self._hopSize
            frame_count += 1

        result = filtered_output * (2 ** 15)
        wavfile.write(outputfile, fs, result.astype(np.int16))

        if en_plot:
            fig, axs = plt.subplots(4)
            axs[0].plot(x)
            axs[1].plot(buf_frame_eng)
            axs[2].plot(buf_frame_eng_hpf)
            axs[3].plot(buf_frame_detected)
            plt.show()


if __name__ == "__main__":
    remover = ClickRemover(1024, 0.01, 0.005)

    file_list = glob.glob("./wavefiles/De_camera_cut_noise(ThermalScan)/*.wav")
    for inputfile in file_list:
        outputfile = os.path.join("./processed", os.path.basename(inputfile))
        remover.process(inputfile, outputfile)


    # ba1 = np.array([ 0.32483446, -0.64966892,  0.32483446, -0.65710985, 0.41692846])
    # ba2 = np.array([ 1,        -2,          1,          1,         -1.62913992, 0.9105507 ])
    #
    # biquad1 = BiquadFilter(ba1)
    # biquad2 = BiquadFilter(ba2)
    #
    # fs, audio = wavfile.read("./wavefiles/De_camera_cut_noise(ThermalScan)/wav_1.wav")
    #
    # start, end = 0, 0
    # frameSize = 1024
    # hopSize = frameSize // 2
    # output = np.zeros(len(audio))
    # while end < len(audio):
    #     end = start + frameSize
    #     x = audio[start:end]
    #     y1 = biquad1.process(x)
    #     output[start:end] = biquad2.process(y1)
    #     start += hopSize
    #
    # wavfile.write("biquad_py.wav", fs, output.astype(np.int16))

