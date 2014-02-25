# -*- coding: utf-8 -*-
"""
@author: adam
"""
import matplotlib.pyplot as plt
import matplotlib as mpl
import langmuir as lm
import numpy as np
import argparse
import os

desc = """
Plot output from fft.py
"""

def get_arguments(args=None):
    parser = argparse.ArgumentParser()
    parser.description = desc

    parser.add_argument(dest='ifile', default='fft.pkl', type=str, nargs='?',
                        metavar='input', help='input file')

    parser.add_argument('--stub', default='', type=str, metavar='stub',
                        help='output file stub')
    parser.add_argument('--ext', default='png', type=str, metavar='str',
        choices=['png', 'pdf', 'jpg'], help='output file type')

    parser.add_argument('--figure', default=(6.0, 6.0, 1.0, 1.0, 1.0, 1.0),
        help='figure dimensions (w, h, l, r, t, b)')

    parser.add_argument('--k'  , action='store_true', help='k stick plot')
    parser.add_argument('--u'  , action='store_true', help='u stick plot')
    parser.add_argument('--v'  , action='store_true', help='v stick plot')
    parser.add_argument('--w'  , action='store_true', help='w stick plot')
    parser.add_argument('--p'  , action='store_true', help='power')
    parser.add_argument('--all', action='store_true', help='plot all')

    parser.add_argument('--title', default='')
    parser.add_argument('--dunits', default='?', type=str,
        help='real space units')

    parser.add_argument('--klim', default=(0, None), nargs=2, type=float)
    parser.add_argument('--kmax', default=None, type=float)
    parser.add_argument('--kmin', default=None, type=float)

    parser.add_argument('--ulim', default=(None, None), nargs=2, type=float)
    parser.add_argument('--umax', default=None, type=float)
    parser.add_argument('--umin', default=None, type=float)

    parser.add_argument('--vlim', default=(None, None), nargs=2, type=float)
    parser.add_argument('--vmax', default=None, type=float)
    parser.add_argument('--vmin', default=None, type=float)

    parser.add_argument('--wlim', default=(None, None), nargs=2, type=float)
    parser.add_argument('--wmax', default=None, type=float)
    parser.add_argument('--wmin', default=None, type=float)

    parser.add_argument('--kmult', default=None, type=float, help='tick mult')
    parser.add_argument('--umult', default=None, type=float, help='tick mult')
    parser.add_argument('--vmult', default=None, type=float, help='tick mult')
    parser.add_argument('--wmult', default=None, type=float, help='tick mult')

    parser.add_argument('--kmaxn', default=4, type=int, help='tick maxn')
    parser.add_argument('--umaxn', default=4, type=int, help='tick maxn')
    parser.add_argument('--vmaxn', default=4, type=int, help='tick maxn')
    parser.add_argument('--wmaxn', default=4, type=int, help='tick maxn')

    parser.add_argument('--fontsize' , default='large')
    parser.add_argument('--labelsize', default=None)
    parser.add_argument('--ticksize' , default=None)
    parser.add_argument('--titlesize', default=None)

    parser.add_argument('--show', action='store_true', help='show plot')
    parser.add_argument('--save', action='store_true', help='save plot')

    opts = parser.parse_args(args)

    if opts.titlesize is None: opts.titlesize = opts.fontsize
    if opts.labelsize is None: opts.labelsize = opts.fontsize
    if opts.ticksize  is None: opts.ticksize  = opts.fontsize

    if opts.umin: opts.ulim[0] = opts.umin
    if opts.umax: opts.ulim[1] = opts.umax

    if opts.vmin: opts.vlim[0] = opts.vmin
    if opts.vmax: opts.vlim[1] = opts.vmax

    if opts.wmin: opts.wlim[0] = opts.wmin
    if opts.wmax: opts.wlim[1] = opts.wmax

    if opts.kmin: opts.klim[0] = opts.kmin
    if opts.kmax: opts.klim[1] = opts.kmax

    opts.umin, opts.umax = opts.ulim
    opts.vmin, opts.vmax = opts.vlim
    opts.wmin, opts.wmax = opts.wlim
    opts.kmin, opts.kmax = opts.klim

    if opts.all:
        opts.k = True
        opts.u = True
        opts.v = True
        opts.w = True
        opts.p = True

    popts = [opts.k, opts.u, opts.v, opts.w, opts.p]
    if not any(popts):
        raise RuntimeError, 'must use --k, --p, etc to chose plot type'

    if not opts.show and not opts.save:
        opts.show = True

    return opts

def calculate_bins(array, width, amin, amax):
    return np.arange(amin, amax + 1.1*width, width)

def fix_limits(array, amin=None, amax=None):
    if amin is None and amax is None:
        amin = np.amin(array)
        amax = np.amax(array)
        if abs(amin) > abs(amax):
            amax = abs(amin)
        else:
            amin = -abs(amax)

    if amin is None:
        amin = np.amin(array)
    if amax is None:
        amax = np.amax(array)

    arng = abs(amax - amin)
    if arng <= 0:
        amin = -1
        amax = +1

    return amin, amax

def plot_power():
    fig, ax1 = lm.plot.subplots(1, 1, 6, 6, 1.5, 1, 1, 1.5)

    cmap = mpl.cm.get_cmap('hot')
    lm.plot.contourf(u, v, p, 128, cmap=cmap)

    lm.plot.title(opts.title, fontsize=opts.titlesize)
    plt.xlabel('$u = 2\pi\~u$ $(%s^{-1})$' % opts.dunits,
               fontsize=opts.labelsize)
    plt.ylabel('$v = 2\pi\~v$ $(%s^{-1})$' % opts.dunits,
               fontsize=opts.labelsize)
    plt.tick_params(labelsize=opts.ticksize)

    plt.xlim(*opts.ulim)
    plt.ylim(*opts.vlim)

    lm.plot.maxn_locator(x=opts.umaxn, y=opts.vmaxn)

    if opts.umult:
        lm.plot.multiple_locator(x=opts.umult)
    if opts.vmult:
        lm.plot.multiple_locator(y=opts.vmult)

    ax1.yaxis.get_offset_text().set_size(opts.ticksize)

    if opts.save:
        handle = lm.common.format_output(stub=opts.stub, name='power',
            ext=opts.ext)
        print 'saved: %s' % handle
        lm.plot.save(handle)

def plot_stick(values, name, xlabel='', xlim=[None, None], **kwargs):
    fig, ax1 = lm.plot.subplots(1, 1, 6, 6, 1.5, 1, 1, 1.5)

    bins = calculate_bins(values, 0.01, *xlim)
    xmin, xmax = np.amin(bins), np.amax(bins)

    _kwargs = dict(bins=bins, histtype='stepfilled', lw=0.0)
    _kwargs.update(**kwargs)
    cs, es, ps = plt.hist(values.flat, weights=p.flat, **_kwargs)

    lm.plot.title(opts.title, fontsize=opts.titlesize)
    plt.xlabel(xlabel, fontsize=opts.labelsize)

    plt.tick_params(labelsize=opts.ticksize)

    xy = ps[0].get_xy()
    xy[:,1] /= np.amax(xy[:,1])
    ps[0].set_xy(xy)

    plt.ylim(-0.1, 1.1)
    plt.xlim(xmin, xmax)

    plt.axhline(0, color='k', lw=1.5, ls='-')

    lm.plot.maxn_locator(x=opts.umaxn, y=opts.vmaxn)

    if opts.umult:
        lm.plot.multiple_locator(x=opts.umult)
    if opts.vmult:
        lm.plot.multiple_locator(y=opts.vmult)

    ax1.yaxis.get_offset_text().set_size(opts.ticksize)

    if opts.save:
        handle = lm.common.format_output(stub=opts.stub, name=name,
            ext=opts.ext)
        print 'saved: %s' % handle
        lm.plot.save(handle)

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    data = lm.common.load_pkl(opts.ifile)
    x = data['x']
    y = data['y']
    z = data['z']
    s = data['s']
    u = data['u']
    v = data['v']
    w = data['w']
    p = data['p']
    f = data['f']

    two_pi = 2.0 * np.pi

    u = two_pi * u
    v = two_pi * v
    w = two_pi * w
    k = np.sqrt(u**2 + v**2 + w**2)

    opts.klim = fix_limits(k, *opts.klim)
    opts.ulim = fix_limits(u, *opts.ulim)
    opts.vlim = fix_limits(v, *opts.vlim)
    opts.wlim = fix_limits(w, *opts.wlim)

    opts.kmin, opts.kmax = opts.klim
    opts.umin, opts.umax = opts.ulim
    opts.vmin, opts.vmax = opts.vlim
    opts.wmin, opts.wmax = opts.wlim

    if opts.p:
        plot_power()

    if opts.k:
        plot_stick(k, 'k', '$k = 2\pi\~k$ $(%s^{-1})$' % opts.dunits,
            xlim=opts.klim, color=lm.plot.colors.p1)

    if opts.u:
        plot_stick(u, 'u', '$u = 2\pi\~u$ $(%s^{-1})$' % opts.dunits,
                   xlim=opts.ulim, color=lm.plot.colors.r1)

    if opts.v:
        plot_stick(v, 'v', '$v = 2\pi\~v$ $(%s^{-1})$' % opts.dunits,
                   xlim=opts.vlim, color=lm.plot.colors.b1)

    if opts.w:
        plot_stick(w, 'w', '$w = 2\pi\~w$ $(%s^{-1})$' % opts.dunits,
                   xlim=opts.wlim, color=lm.plot.colors.g2)

    if opts.show:
        plt.show()
