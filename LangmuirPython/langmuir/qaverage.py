# -*- coding: utf-8 -*-
import numpy as np
import warnings

def qaverage(q, values, nbins=100, bmin=None, bmax=None, weights=None,
             interp=True, left=0, right=0, action='ignore'):
    """
    Put q's into bins and then average values based on their q-bin.

    :param q: values to bin
    :param values: values to average
    :param nbins: number of bins
    :param bmin: lower bin value, default = min(q)
    :param bmin: upper bin value, default = max(q)
    :param weights: weight the average
    :param interp: interpolate NaN
    :param left: left interp default
    :param right: right interp default
    :param action: warnings filter action
    """
    assert q.size == values.size

    # bmin
    if bmin is None:
        bmin = np.amin(q)

    # bmax
    if bmax is None:
        bmax = np.amax(q)

    assert bmax > bmin

    # bins
    bins = np.linspace(bmin, bmax, nbins)

    # histogram (indices into bin array that each q-value falls)
    whichbin = np.digitize(q.flat, bins)

    # image avg and std
    i_avg = []
    i_std = []

    with warnings.catch_warnings():
        warnings.simplefilter(action, RuntimeWarning)

        # average image values
        if weights is None:
            for b in range(1, nbins + 1):
                s = (whichbin == b)
                i_avg.append(values.flat[s].mean())
                i_std.append(values.flat[s].std())

        else:
            assert weights.size == values.size

            for b in range(1, nbins + 1):
                s = (whichbin == b)
                w = weights.flat[s]
                i = values.flat[s]

                i_avg.append(np.average(i, weights=w))
                i_std.append(np.sqrt(np.average((i - i_avg[-1])**2, weights=w)))

    i_avg = np.array(i_avg)
    i_std = np.array(i_std)

    if interp:
        not_nan = np.logical_not(np.isnan(i_avg))
        i_avg = np.interp(bins, bins[not_nan], i_avg[not_nan], left=0, right=0)
        i_std = np.interp(bins, bins[not_nan], i_std[not_nan], left=0, right=0)

    return bins, i_avg, i_std

def azimuthal_average(x, y, image, ox=0, oy=0, **kwargs):
    """
    Average image along radial contours.

    :param x: x-values
    :param y: y-values
    :param image: image values
    :param ox: x-origin
    :param oy: y-origin
    """
    # limits
    xmin, xmax = np.amin(x), np.amax(x)
    ymin, ymax = np.amin(y), np.amax(y)

    # origin
    if ox is None:
        ox = 0.5 * (xmax - xmin)

    if oy is None:
        oy = 0.5 * (ymax - ymin)

    # radial values
    r = np.sqrt((x - ox)**2 + (y - oy**2)**2)

    # average image based on r-bins
    return qaverage(r, image, **kwargs)

def radial_average(x, y, image, ox=0, oy=0, **kwargs):
    """
    Average image along rays from origin.

    :param x: x-values
    :param y: y-values
    :param image: image values
    :param ox: x-origin
    :param oy: y-origin
    """
    # limits
    xmin, xmax = np.amin(x), np.amax(x)
    ymin, ymax = np.amin(y), np.amax(y)

    # origin
    if ox is None:
        ox = 0.5 * (xmax - xmin)

    if oy is None:
        oy = 0.5 * (ymax - ymin)

    # theta values
    t = np.arctan2(y - oy, x - ox)

    _kwargs = dict(bmin=0, bmax=2 * np.pi)
    _kwargs.update(**kwargs)

    # average image based on r-bins
    return qaverage(t, image, **_kwargs)