import argparse
import numpy as np
import matplotlib.pyplot as plt


def dB2lin(value):
    return pow(10.0, 0.05 * value)


def lin2dB(value):
    return 20.0 * np.log10(value)


def knee_curve(x, k, linear_threshold, offset=0):
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


def compute_curve(x_data, pregain=0.0, postgain=0.0,
                  threshold=-24.0, knee=1.0, ratio=2.0,
                  threshold_agg=-12.0, knee_agg=1.0, ratio_agg=4.0,
                  threshold_noise=-60.0, knee_noise=1.0, ratio_noise=0.5):
    """
    x_data: input arary, linear data [1/32767, 0)
    y_data: output arary, linear data [1/32767, 0)
    """

    slope = 1 / ratio
    slope_agg = 1 / ratio_agg
    samples = np.size(x_data)
    y_data = np.zeros(samples)
    linear_threshold = dB2lin(threshold)

    # pre-gain
    x_data = x_data * dB2lin(pregain)

    # compressor 1 (normal)
    k = calculate_k(threshold, knee, slope)
    linear_thresholdknee = dB2lin(threshold + knee)
    kneedBoffset = lin2dB(knee_curve(linear_thresholdknee, k, linear_threshold))

    # compressor 2 (aggressive)
    linear_threshold_agg = dB2lin(threshold_agg)
    linear_thresholdknee_agg = dB2lin(threshold_agg + knee_agg)
    k_agg = calculate_k(threshold_agg, knee_agg, slope_agg)
    kneedBoffset_agg = lin2dB(knee_curve(dB2lin(threshold_agg + knee_agg),
                                                k_agg, linear_threshold_agg))
    knot_thr_agg = (kneedBoffset + slope * (lin2dB(linear_threshold_agg) - threshold - knee))
    offset_agg = threshold_agg - knot_thr_agg

    # TODO:
    # 1) add soft-knee for noise gate
    # 2) re-design the knee_curve of compressor 2

    # noise gate
    slope_noise = 1 / ratio_noise
    linear_threshold_noise = dB2lin(threshold_noise)
    knee_noise = 0

    for i in range(samples):
        x = x_data[i]

        if x < linear_threshold:
            if x > linear_threshold_noise:
                # linear piece
                y_data[i] = x
            else:
                # noise gate
                y_data[i] = dB2lin(max(threshold_noise - knee_noise - slope_noise * (threshold_noise - lin2dB(x)), -93))

        else:
            # compressor 1
            if x < linear_threshold_agg:
                if x > linear_thresholdknee:
                    y_data[i] = dB2lin(kneedBoffset + slope * (lin2dB(x) - threshold - knee))
                else:
                    y_data[i] = knee_curve(x, k, linear_threshold)

            # compressor 2 (aggressive)
            else:
                if x > linear_thresholdknee_agg:
                    y_data[i] = dB2lin(kneedBoffset_agg +
                                       slope_agg * (lin2dB(x) - threshold_agg - knee_agg) - offset_agg)
                else:
                    y_data[i] = knee_curve(x, k_agg, linear_threshold_agg) / dB2lin(offset_agg)

    # post-gain
    y_data = y_data * dB2lin(postgain)

    return y_data


def plot_figure(x_data, pregain=0.0, postgain=0.0,
                threshold=-24.0, knee=3.0, ratio=2.0,
                threshold_agg=-12.0, knee_agg=3.0, ratio_agg=4.0,
                threshold_noise=-40, knee_noise=3.0, ratio_noise=0.5):
    """
    x_data: arary, linear data [1/32767, 0)
    y_data: arary, linear data [1/32767, 0)
    """

    dBFS_max = 5
    dBFS_min = -96

    x_dB = lin2dB(x_data)
    y_dB = lin2dB(y_data)

    # plot figure
    plt.rcParams["figure.figsize"] = (12, 9.6)
    plt.plot(x_dB, y_dB)
    plt.xlim([dBFS_min, dBFS_max])
    plt.xlabel("input dBFS")
    plt.xticks(np.arange(-96, 3, 3))

    plt.ylim([dBFS_min, dBFS_max])
    plt.ylabel("output dBFS")
    plt.yticks(np.arange(-96, 3, 3))

    # reference line
    plt.plot(x_dB, x_dB, 'r--')

    plt.title("Dynamic Range Compressor\n"
              "pre-gain: {:2.2f}, post-gain {:2.2f}\n"
              "threshold_noise: {:2.2f}, ratio_noise: {:2.2f}\n"
              "threshold: {:2.2f}, knee: {:2.2f}, ratio: {:2.2f}\n"
              "threshold_agg: {:2.2f}, knee_agg: {:2.2f}, ratio_agg: {:2.2f}".format(
                  pregain, postgain,
                  threshold_noise, ratio_noise,
                  threshold, knee, ratio,
                  threshold_agg, knee_agg, ratio_agg))
    plt.grid()
    plt.show()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Dynamic Range Compressor Curve")
    parser.add_argument("--pregain", type=float, default=0.0)
    parser.add_argument("--postgain", type=float, default=0.0)

    # compressor 1
    parser.add_argument("--threshold", type=float, default=-24.0)
    parser.add_argument("--knee", type=float, default=1.0)
    parser.add_argument("--ratio", type=float, default=2.0)

    # compressor 2
    parser.add_argument("--threshold_agg", type=float, default=-12.0)
    parser.add_argument("--knee_agg", type=float, default=1.0)
    parser.add_argument("--ratio_agg", type=float, default=4.0)

    # noise gate
    parser.add_argument("--threshold_noise", type=float, default=-60.0)
    parser.add_argument("--knee_noise", type=float, default=1.0)
    parser.add_argument("--ratio_noise", type=float, default=0.5)

    args = parser.parse_args()

    x_int16 = np.arange(1, 32768, 1, dtype=float)
    x_data = x_int16 / 32767.0
    y_data = compute_curve(x_data,
                           pregain=args.pregain,
                           postgain=args.postgain,
                           threshold=args.threshold,
                           knee=args.knee,
                           ratio=args.ratio,
                           threshold_agg=args.threshold_agg,
                           knee_agg=args.knee_agg,
                           ratio_agg=args.ratio_agg,
                           threshold_noise=args.threshold_noise,
                           knee_noise=args.knee_noise,
                           ratio_noise=args.ratio_noise)

    plot_figure(x_data,
                pregain=args.pregain,
                postgain=args.postgain,
                threshold=args.threshold,
                knee=args.knee,
                ratio=args.ratio,
                threshold_agg=args.threshold_agg,
                knee_agg=args.knee_agg,
                ratio_agg=args.ratio_agg,
                threshold_noise=args.threshold_noise,
                knee_noise=args.knee_noise,
                ratio_noise=args.ratio_noise)
