import argparse
import numpy as np
import matplotlib.pyplot as plt


def dB2lin(value):
    return pow(10.0, 0.05 * value)


def lin2dB(value):
    return 20.0 * np.log10(value)


def knee_curve(x, k, linear_threshold):
    return linear_threshold + (1.0 - np.exp(-k * (x - linear_threshold))) / k

def knee_slope(x, k, linear_threshold):
    """
    This function is courtesy of http://www.derivative-calculator.net/
    it is the derivative of lin2db(kneecurve(db2lin(x))), simplified a bit and made linear
    """
    return k * x / ((k * linear_threshold + 1.0) * np.exp(k * (x - linear_threshold)) - 1)


def calculate_k(threshold, knee, slope):
    """
    the knee curve is dialed into the correct shape using the `k` value
    our goal is that the slope of the knee curve should match the desired slope at the exact
    point where it transitions from the knee to the downward compression defined by ratio
    """
    xknee = dB2lin(threshold + knee)
    linear_threshold = dB2lin(threshold)
    mink = 0.1
    maxk = 10000.0
    k = 5  # initial guess

    for i in range(15):
        if knee_slope(xknee, k, linear_threshold) < slope:
            maxk = k
        else:
            mink = k
        k = np.sqrt(mink * maxk)

    return k


def compute_curve(x_data, pregain=0.0, postgain=0.0, threshold=-24.0, knee=3.0, ratio=4.0):
    """
    x_data: input arary, linear data [1/32767, 0)
    y_data: output arary, linear data [1/32767, 0)
    """

    slope = 1 / ratio
    samples = np.size(x_data)
    y_data = np.zeros(samples)
    linear_threshold = dB2lin(threshold)

    x_data = x_data * dB2lin(pregain)

    if knee <= 0:
        # no knee curve case
        for i in range(samples):
            x = x_data[i]
            if x < linear_threshold:
                y_data[i] = x
            else:
                y_data[i] = dB2lin(threshold + slope * (lin2dB(x) - threshold))
    else:
        # knee curve case
        k = calculate_k(threshold, knee, slope)
        linear_thresholdknee = dB2lin(threshold + knee)
        kneedBoffset = lin2dB(knee_curve(dB2lin(threshold + knee), k, linear_threshold))
        # print("linear_threshold: {}, knee dBOffset: {}".format(linear_thresholdknee, kneedBoffset))

        for i in range(samples):
            x = x_data[i]
            if x < linear_threshold:
                y_data[i] = x
            elif x < linear_thresholdknee:
                y_data[i] = knee_curve(x, k, linear_threshold)
            else:
                y_data[i] = dB2lin(kneedBoffset + slope * (lin2dB(x) - threshold - knee))

    y_data = y_data * dB2lin(postgain)

    return y_data



def plot_figure(x_data, y_data, pregain, postgain, threshold, knee, ratio):
    """
    x_data: arary, linear data [1/32767, 0)
    y_data: arary, linear data [1/32767, 0)
    """

    head_dBFS = 5
    x_dB = lin2dB(x_data)
    y_dB = lin2dB(y_data)

    # plot figure
    plt.rcParams["figure.figsize"] = (12, 9.6)
    plt.plot(x_dB, y_dB)
    plt.xlim([-96, head_dBFS])
    plt.xlabel("input dBFS")
    plt.xticks(np.arange(-96, 3, 3))

    plt.ylim([-96, head_dBFS])
    plt.ylabel("output dBFS")
    plt.yticks(np.arange(-96, 3, 3))

    plt.title("Dynamic Range Compressor\n pre-gain: {:2.2f}, postgain {:2.2f}\n threshold: {:2.2f}, knee: {:2.2f}, ratio: {:2.2f}".format(pregain, postgain, threshold, knee, ratio))
    plt.grid()
    plt.show()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Dynamic Range Compressor Curve")
    parser.add_argument("--pregain", type=float, default=0.0)
    parser.add_argument("--postgain", type=float, default=0.0)
    parser.add_argument("--threshold", type=float, default=-24.0)
    parser.add_argument("--knee", type=float, default=3.0)
    parser.add_argument("--ratio", type=float, default=4.0)

    args = parser.parse_args()

    x_int16 = np.arange(1, 32768, 1, dtype=float)
    x_data = x_int16 / 32767.0
    y_data = compute_curve(x_data, args.pregain, args.postgain, args.threshold, knee=args.knee, ratio=args.ratio)
    plot_figure(x_data, y_data, args.pregain, args.postgain, args.threshold, args.knee, args.ratio)
