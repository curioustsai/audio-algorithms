import numpy as np
from scipy import signal
# import matplotlib.pyplot as plt


"""
high pass filter coeffiecents
"""

cutoff_freqs = [100, 150, 200, 250, 300, 400, 500]
sample_rates = [8000, 16000, 32000, 48000]
num_biquad = 2
num_coefs = 5
# [b0, b1, b2, a1, a2]
hpf_coefficents = np.zeros([len(sample_rates), len(cutoff_freqs), num_biquad, num_coefs])

# high pass filter coefficients
for idx_fs, fs in enumerate(sample_rates):
    fw = fs / 2
    for idx_fc, fc in enumerate(cutoff_freqs):
        sos = signal.cheby1(4, 3, fc / fw, 'high', output='sos')
        hpf_coefficents[idx_fs, idx_fc, :, :] = np.delete(sos, [3], axis=1)

print("hpf_coefficents")
print(repr(hpf_coefficents))


"""
low pass filter coeffiecents
"""
cutoff_freqs = [4000, 5000, 6000, 7000]
sample_rates = [16000, 32000, 48000]
num_biquad = 2
num_coefs = 5
# [b0, b1, b2, a1, a2]
lpf_coefficents = np.zeros([len(sample_rates), len(cutoff_freqs), num_biquad, num_coefs])

# low pass filter coefficients
for idx_fs, fs in enumerate(sample_rates):
    fw = fs / 2
    for idx_fc, fc in enumerate(cutoff_freqs):
        sos = signal.cheby1(4, 3, fc / fw, 'low', output='sos')
        lpf_coefficents[idx_fs, idx_fc, :, :] = np.delete(sos, [3], axis=1)

print("lpf_coefficents")
print(repr(lpf_coefficents))
