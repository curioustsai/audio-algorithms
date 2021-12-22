# 1. generate noise from random seed
# 2. pass through LPF, freqz([1, 0.75], [1, -0.75])


import numpy as np
from scipy import signal
import matplotlib.pyplot as plt

b = [1, 0.75]
a = [1, -0.75]
freq, h = signal.freqz(b, a, worN=2048, fs=48000)

fig, ax = plt.subplots(2, 1, figsize=(8, 6))
ax[0].plot(freq, 20*np.log10(abs(h)), color='blue')
ax[0].set_title("Frequency Response")
ax[0].set_ylabel("Amplitude (dB)", color='blue')
ax[0].grid()

ax[1].plot(freq, np.unwrap(np.angle(h))*180/np.pi, color='green')
ax[1].set_ylabel("Angle (degrees)", color='green')
ax[1].set_xlabel("Frequency (Hz)")
ax[1].grid()
plt.show()


# # generate a noisy singal
# rng = np.random.default_rng()
# t = np.linspace(-1, 1, 201)
# x = (np.sin(2*np.pi*0.75*t*(1-t) + 2.1) +
#           0.1*np.sin(2*np.pi*1.25*t + 1) +
#           0.18*np.cos(2*np.pi*3.85*t))
# xn = x + rng.standard_normal(len(t)) * 0.08
#
# y = signal.filtfilt(b, a, xn)
# plt.figure()
# plt.plot(t, xn, 'b', alpha=0.75)
# plt.plot( t, y, 'k')
# plt.legend(('noisy signal', 'lfilter, once', 'lfilter, twice',
#                         'filtfilt'), loc='best')
# plt.grid(True)
# plt.show()
