import numpy as np
from scipy import signal
import matplotlib.pyplot as plt

# sos = signal.cheby1(4, 3, 4000/24000, 'low', output='sos')
# sos = signal.cheby1(4, 3, 200/24000, 'low', output='sos')
sos = signal.cheby1(4, 3, 200/24000, 'high', output='sos')
# sos = signal.cheby1(4, 3, 500/24000, 'low', output='sos')
# sos = signal.cheby1(4, 3, 10000/24000, 'high', output='sos')
# sos = signal.cheby1(4, 3, [500/24000, 1000/24000], 'bandstop', output='sos')

# sos = signal.cheby1(2, 3, [500/24000, 2000/24000], 'bandpass', output='sos')
# sos = signal.cheby1(2, 3, [500/24000, 800/24000], 'bandstop', output='sos')
# sos = signal.cheby1(4, 3, 7000/24000, 'low', output='sos')
# sos = signal.cheby1(2, 3, [500/24000, 800/24000], 'bandpass', output='sos')
print(sos)
w, h = signal.sosfreqz(sos, worN=1024)

plt.subplot(2, 1, 1)
db = 20*np.log10(np.maximum(np.abs(h), 1e-5))
plt.plot(w/np.pi, db)
plt.ylim(-75, 5)
plt.grid(True)
plt.yticks([0, -20, -40, -60])
plt.ylabel('Gain [dB]')
plt.title('Frequency Response')
plt.subplot(2, 1, 2)
plt.plot(w/np.pi, np.angle(h))
plt.grid(True)
plt.yticks([-np.pi, -0.5*np.pi, 0, 0.5*np.pi, np.pi],
                      [r'$-\pi$', r'$-\pi/2$', '0', r'$\pi/2$', r'$\pi$'])
plt.ylabel('Phase [rad]')
plt.xlabel('Normalized frequency (1.0 = Nyquist)')
plt.show()
