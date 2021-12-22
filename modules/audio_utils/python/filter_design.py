import tkinter
import numpy as np
from scipy import signal
from scipy.io import wavfile
import matplotlib.pyplot as plt
import matplotlib
matplotlib.use('TkAgg')


def peak_eq(f0, gain=0, Q=1, fs=48000):
    """
    param f0: center frequency
    param gain: gain > 0 peak filter, gain < 0 notch filter
    param Q: peak bandwidth
    param fs: sample rate
    return biqaud coefficent
    """

    A = np.sqrt(10 ** (gain / 20))
    w0 = 2 * np.pi * f0 / fs
    alpha = np.sin(w0) / (2 * Q)

    b0 = 1 + alpha * A
    b1 = -2 * np.cos(w0)
    b2 = 1 - alpha * A
    a0 = 1 + alpha / A
    a1 = -2 * np.cos(w0)
    a2 = 1 - alpha / A
    b = np.array([b0, b1, b2]) / a0
    a = np.array([a0, a1, a2]) / a0

    print("b cofficient {}".format(b))
    print("a cofficient {}".format(a))

    return b, a

    # freq, h = signal.freqz(b, a, worN=2048)
    #
    # return freq, h


def peakeq(f0=500, Q=30, fs=48000):
    b, a = signal.iirpeak(f0, Q, fs)
    freq, h = signal.freqz(b, a, worN=2048)

    return freq, h


def notch(f0=500, Q=30, fs=48000):
    b, a = signal.iirnotch(f0, Q, fs)
    freq, h = signal.freqz(b, a, worN=2048)

    return freq, h


def plotFreqResponse(freq, h):
    fig, ax = plt.subplots(2, 1, figsize=(8, 6))
    ax[0].plot(freq, 20*np.log10(np.maximum(abs(h), 1e-5)), color='blue')
    ax[0].set_title("Frequency Response")
    ax[0].set_ylabel("Amplitude (dB)", color='blue')
    # ax[0].set_xlim([0, 48000])
    # ax[0].set_ylim([-50, 10])
    ax[0].grid()
    ax[1].plot(freq, np.unwrap(np.angle(h))*180/np.pi, color='green')
    ax[1].set_ylabel("Angle (degrees)", color='green')
    ax[1].set_xlabel("Frequency (Hz)")
    # ax[1].set_xlim([0, 48000])
    # ax[1].set_yticks([-90, -60, -30, 0, 30, 60, 90])
    # ax[1].set_ylim([-90, 90])
    ax[1].grid()
    plt.show()


if __name__ == "__main__":
    # freq, h = peak_eq(f0=500, gain=3, Q=1, fs=48000)
    # freq, h = peak_eq(f0=500, gain=0, Q=1, fs=48000)
    b, a = peak_eq(f0=500, gain=-3, Q=1, fs=48000)

    fs, data = wavfile.read("./sweeping_tone.wav")
    y = signal.lfilter(b, a, data)
    wavfile.write('output.wav', fs, y.astype(np.int16))
