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
                  threshold_expander=-60.0, knee_expander=1.0, ratio_expander=4.0):
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
    # 1) add soft-knee for expander
    # 2) re-design the knee_curve of compressor 2

    # expander
    slope_expander = 1 / ratio_expander
    linear_threshold_expander = dB2lin(threshold_expander)
    knee_expander = 0

    for i in range(samples):
        x = x_data[i]

        if x < linear_threshold:
            if x > linear_threshold_expander:
                # linear piece
                y_data[i] = x
            else:
                # expander
                y_data[i] = dB2lin(max(threshold_expander - knee_expander - slope_expander * (threshold_expander - lin2dB(x)), -93))

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
                threshold_expander=-40, knee_expander=3.0, ratio_expander=0.5):
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
    # plt.plot(x_dB, x_dB, 'r--')
    #
    # xx = np.array([-93.00, -90.00, -87.00, -84.00, -81.00, -78.00, -75.00, -72.00, -69.00, -66.00, -63.00, -60.00, -57.00, -54.00, -51.00, -48.00, -45.00, -42.00, -39.00, -36.00, -33.00, -30.00, -27.00, -24.00, -21.00, -18.00, -15.00, -12.00, -9.00, -6.00, -3.00, 0.00, ])
    # yy = np.array([-68.08, -67.33, -66.58, -65.83, -65.08, -64.33, -63.58, -62.83, -62.08, -61.33, -60.58, -59.83, -56.95, -53.95, -50.96, -47.98, -44.98, -42.00, -39.00, -36.00, -33.00, -30.00, -27.00, -24.00, -22.21, -20.70, -19.19, -17.68, -16.58, -15.77, -15.01, -14.26, ])
    #
    # plt.plot(xx, yy, 'g--')

    plt.title("Dynamic Range Compressor\n"
              "pre-gain: {:2.2f}, post-gain {:2.2f}\n"
              "threshold_expander: {:2.2f}, ratio_expander: {:2.2f}\n"
              "threshold: {:2.2f}, knee: {:2.2f}, ratio: {:2.2f}\n"
              "threshold_agg: {:2.2f}, knee_agg: {:2.2f}, ratio_agg: {:2.2f}".format(
                  pregain, postgain,
                  threshold_expander, ratio_expander,
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

    # expander
    parser.add_argument("--threshold_expander", type=float, default=-60.0)
    parser.add_argument("--knee_expander", type=float, default=1.0)
    parser.add_argument("--ratio_expander", type=float, default=4.0)

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
                           threshold_expander=args.threshold_expander,
                           knee_expander=args.knee_expander,
                           ratio_expander=args.ratio_expander)

    plot_figure(x_data,
                pregain=args.pregain,
                postgain=args.postgain,
                threshold=args.threshold,
                knee=args.knee,
                ratio=args.ratio,
                threshold_agg=args.threshold_agg,
                knee_agg=args.knee_agg,
                ratio_agg=args.ratio_agg,
                threshold_expander=args.threshold_expander,
                knee_expander=args.knee_expander,
                ratio_expander=args.ratio_expander)
