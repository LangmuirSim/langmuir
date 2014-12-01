import matplotlib.pyplot as plt
import matplotlib as mpl
import numpy as np

cmapRY = mpl.colors.ListedColormap([(1, 0, 0), (1, 1, 0)])

def subplots(nrows=1, ncols=1, w=4, h=4, l=1, r=1, b=1, t=1, ws=1, hs=1):
    fw = float(l + ncols * w + (ncols - 1) * ws + r)
    fh = float(b + nrows * h + (nrows - 1) * hs + t)
    l  = l / fw
    r  = (fw - r) / fw
    b  = b / fh
    t  = (fh - t) / fh
    ws = float(ws) / w
    hs = float(hs) / h
    fig, axes = plt.subplots(nrows, ncols, figsize=(fw, fh))
    plt.subplots_adjust(left=l, right=r, bottom=b, top=t, wspace=ws, hspace=hs)
    return fig, axes

def format_plot(ax1):
    plt.sca(ax1)
    ax1.xaxis.set_major_locator(mpl.ticker.MultipleLocator(32))
    ax1.yaxis.set_major_locator(mpl.ticker.MultipleLocator(32))
    plt.tick_params(direction='out', label1On=False)

def set_limits(ax1, image, xmin=None, xmax=None, ymin=None, ymax=None):
    plt.sca(ax1)
    if xmin is None: xmin = 0
    if xmax is None: xmax = image.shape[0]
    if ymin is None: ymin = 0
    if ymax is None: ymax = image.shape[1]
    plt.xlim(xmin, xmax)
    plt.ylim(ymin, ymax)

def pcolormesh(image, ax1=None, **kwargs):
    if ax1 is None:
        fig, ax1 = subplots(1, 1)
    plt.sca(ax1)
    _kwargs = dict(cmap=cmapRY)
    _kwargs.update(**kwargs)
    plt.pcolormesh(image, **_kwargs)
    format_plot(ax1)
    set_limits(ax1, image)

def imshow(image, ax1=None, **kwargs):
    if ax1 is None:
        fig, ax1 = subplots(1, 1)
    plt.sca(ax1)
    _kwargs = dict(cmap=cmapRY)
    _kwargs.update(**kwargs)
    plt.imshow(image, **_kwargs)
    format_plot(ax1)
    set_limits(ax1, image)

def contourf(x, y, image, ax1=None, *args, **kwargs):
    if ax1 is None:
        fig, ax1 = subplots(1, 1)
    plt.sca(ax1)
    _kwargs = dict(cmap='spectral')
    _kwargs.update(**kwargs)
    plt.contour(x, y, image, *args, **kwargs)
    format_plot(ax1)
    set_limits(ax1, image)

def contour(x, y, image, ax1=None, *args, **kwargs):
    if ax1 is None:
        fig, ax1 = subplots(1, 1)
    plt.sca(ax1)
    _kwargs = dict(cmap='spectral')
    _kwargs.update(**kwargs)
    plt.contour(x, y, image, *args, **kwargs)
    format_plot(ax1)
    set_limits(ax1, image)

def compare(i0, i1, **kwargs):
    fig, (ax1, ax2, ax3) = subplots(1, 3)
    pcolormesh(i0, ax1, **kwargs)
    pcolormesh(i1, ax2, **kwargs)
    pcolormesh(np.logical_xor(i0, i1), ax3, **kwargs)