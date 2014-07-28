# -*- coding: utf-8 -*-
import matplotlib.pyplot as plt
import matplotlib.ticker
import matplotlib.colors
import scipy.fftpack as fftpack
from skimage import morphology
import scipy.signal as signal
import scipy.stats as stats
from scipy import ndimage
from scipy import misc
from PIL import Image
import pandas as pd
import numpy as np
import collections
import itertools
import warnings
import argparse
import logging
import os

txt_exts = ['.txt']
img_exts = ['.png', '.jpg', '.jpeg']
all_exts = txt_exts + img_exts

def find_inputs(work, exts):
    """
    Search for input files.
    """
    found = []
    root, dirs, files = os.walk(os.getcwd()).next()
    for f in files:
        stub, ext = os.path.splitext(f)
        if ext.lower() in exts:
            found.append(os.path.join(root, f))
    return found

def get_arguments():
    """
    Get command line arguments.
    """
    parser = argparse.ArgumentParser()

    parser.add_argument(dest='inputs', nargs='*', help='input file names',
        type=str, default=[], metavar='image.png')

    parser.add_argument('--detrend', action='store_true', help='detrend the signal')

    parser.add_argument('--window', action='store_true', help='apply hamming window to signal')

    opts = parser.parse_args()

    # search for images if None were passed
    # if not opts.inputs:
    #    opts.inputs = find_inputs(os.getcwd(), all_exts)

    # make sure images were found
    if not opts.inputs:
        raise RuntimeError('no images found! %s' % ' '.join(all_exts))
    else:
        logging.info('found %d images' % len(opts.inputs))

    # make sure absolute paths are used
    opts.inputs = [os.path.abspath(image) for image in opts.inputs]

    # make sure images exist
    for image in opts.inputs:
        if not os.path.exists(image):
            raise RuntimeError('image does not exist: %s' % image)

    return opts

def load_ascii(handle, shape=None, astype=np.float64, **kwargs):
    """
    Load data from txt file.
    """
    # load the data
    image = np.loadtxt(handle, **kwargs)

    # reshape the data as if NxN
    if shape is None and not len(image.shape) == 2:
        try:
            size = int(np.sqrt(image.size))
            image = np.reshape(image, (size, size))
        except ValueError:
            raise RuntimeError, 'can not reshape data'
    else:
        image = np.reshape(image, shape)

    return image.astype(astype)

def load_image(handle, shape=None, astype=np.float64):
    """
    Load data from image file.
    """
    pil_img = Image.open(handle)
    image = misc.fromimage(pil_img.convert("L"))

    if not shape is None:
        image = np.reshape(image, shape)

    return image.astype(astype)

def load(handle, astype=np.float64):
    """
    Load data from file based on ext.
    """
    stub, ext = os.path.splitext(handle)

    if ext in img_exts:
        return load_image(handle, astype=astype)

    if ext in txt_exts:
        return load_ascii(handle, astype=astype)

    raise RuntimeError('unknown file format: %s' % ext)

class Grid(object):
    """
    Grid of x,y points.
    """
    def __init__(self, image):
        # number of points
        self.nx = int(image.shape[0])
        self.ny = int(image.shape[1])

        # spacing
        self.dx = 1.0
        self.dy = 1.0

        # limits
        self.xmin = 0
        self.ymin = 0

        self.xmax = float(self.nx)
        self.ymax = float(self.ny)

        # lengths
        self.lx = abs(self.xmax - self.xmin)
        self.ly = abs(self.ymax - self.ymin)

        # mesh
        self.y, self.x = np.meshgrid(
            np.fromiter(((0.5 + i) * self.dx for i in range(self.nx)),
                dtype=np.float64, count=self.nx),
            np.fromiter(((0.5 + i) * self.dy for i in range(self.ny)),
                dtype=np.float64, count=self.ny))

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

class FFT():
    """
    Fourier Transform.
    """
    def __init__(self, x, y, s, detrend=True, window=False, **kwargs):
        # r-space
        self.x  = np.asanyarray(x)
        self.y  = np.asanyarray(y)
        self.s  = np.asanyarray(s)

        assert len(self.x.shape) == 2
        assert self.x.shape == self.y.shape == self.s.shape
        assert self.x.size == self.y.size == self.s.size

        # r-space spacing
        self.dx = self._delta(self.x, np.index_exp[0,0], np.index_exp[1,0])
        self.dy = self._delta(self.y, np.index_exp[0,0], np.index_exp[0,1])

        # r-space samples
        self.n0 = self.x.shape[0]
        self.n1 = self.x.shape[1]

        # r-space lengths
        self.lx = self.n0 * self.dx
        self.ly = self.n1 * self.dy

        # k-space
        u = fftpack.fftshift(fftpack.fftfreq(self.n0))
        v = fftpack.fftshift(fftpack.fftfreq(self.n1))
        self.u, self.v = np.meshgrid(u, v, indexing='ij')

        # k-space spacing
        self.du = self._delta(self.u, np.index_exp[0,0], np.index_exp[1,0])
        self.dv = self._delta(self.v, np.index_exp[0,0], np.index_exp[0,1])

        # k-space lengths
        self.lu = self.n0 * self.du
        self.lv = self.n1 * self.dv

        # nyquist
        try:
            self.nyquist_u = 0.5/self.dx
        except ZeroDivisionError:
            self.nyquist_u = 0.0

        try:
            self.nyquist_v = 0.5/self.dy
        except ZeroDivisionError:
            self.nyquist_v = 0.0

        self.k = np.sqrt(self.u**2 + self.v**2)

        # detrend the signal
        if detrend:
            self.s = signal.detrend(self.s)

        # apply window to signal
        if window:
            self._window()
            self.s = self.s * self.window

        # compute the FFT
        self.fft = fftpack.fftshift(fftpack.fft2(self.s))
        self.power = self.fft.real**2 + self.fft.imag**2

    @staticmethod
    def _delta(a, s0, s1):
        a = np.asanyarray(a)
        try:
            delta = abs(a[s1] - a[s0])
        except IndexError:
            delta = 1.0
        return delta

    def _window(self):
            win_u  = signal.hamming(self.u.size)
            win_v  = signal.hamming(self.v.size)
            self.window = np.outer(win_u, win_v)
            self.window = np.reshape(self.window, self.s.shape)

    def __str__(self):
        s  = '[FFT2D]\n'
        s += '  %-6s = %s, %s\n' % ('shape', self.n0, self.n1)
        return s

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    for fname in opts.inputs:
        stub, ext = os.path.splitext(fname)

	print fname

        image = load(fname)
        grid = Grid(image)

	fft = FFT(grid.x, grid.y, image, detrend=opts.detrend, window=opts.window)

	results = collections.OrderedDict()
	results.update(x=fft.x.flat, y=fft.y.flat, i=fft.s.flat, u=fft.u.flat, v=fft.v.flat, k=fft.k.flat,
		       p=fft.power.flat)

	data = pd.DataFrame(results, index=range(fft.x.size), columns=results.keys())

	oname = '{stub}_fft.csv'.format(stub=stub)
	data.to_csv(oname)
