# -*- coding: utf-8 -*-
"""
@author: adam
"""
import matplotlib.pyplot as plt
import langmuir as lm
import numpy as np
import argparse
import os

desc = """
Plot output from rdf.py
"""

def get_arguments(args=None):
    parser = argparse.ArgumentParser()
    parser.description = desc

    parser.add_argument(dest='ifile', default='rdf.pkl', type=str, nargs='?',
                        metavar='input', help='input file')

    parser.add_argument('--stub', default='', type=str, metavar='stub',
                        help='output file stub')
    parser.add_argument('--ext', default='pdf', type=str, metavar='str',
        choices=['png', 'pdf', 'jpg'], help='output file type')

    parser.add_argument('--figure', default=(6.0, 6.0, 1.0, 1.0, 1.0, 1.0),
        help='figure dimensions (w, h, l, r, t, b)')

    parser.add_argument('--r1' , action='store_true', help='r1')
    parser.add_argument('--w1' , action='store_true', help='w1')
    parser.add_argument('--w2' , action='store_true', help='w2')
    parser.add_argument('--d12', action='store_true', help='w2 - w1')
    parser.add_argument('--d21', action='store_true', help='w1 - w2')
    parser.add_argument('--s12', action='store_true', help='w1 + w2')
    parser.add_argument('--all', action='store_true', help='all')

    parser.add_argument('--normalize', action='store_true',
        help='normalize to [0, 1]')

    parser.add_argument('--detrend', action='store_true',
        help='subtract average')

    parser.add_argument('--abs', action='store_true',
        help='take absolute value')

    parser.add_argument('--rscale', type=float, default=1.0, help='scale r1')
    parser.add_argument('--bins', type=int, default=256, help='number of bins')

    parser.add_argument('--title'   , default='')
    parser.add_argument('--xlabel'  , default='$\Delta r (nm)$')
    parser.add_argument('--ylabel'  , default='')

    parser.add_argument('--xlim', default=(None, None), nargs=2, type=float)
    parser.add_argument('--xmax', default=None, type=float)
    parser.add_argument('--xmin', default=None, type=float)

    parser.add_argument('--ylim', default=(None, None), nargs=2, type=float)
    parser.add_argument('--ymax', default=None, type=float)
    parser.add_argument('--ymin', default=None, type=float)

    parser.add_argument('--xmult', default=None, type=float,
        help='xtick multiple')
    parser.add_argument('--ymult', default=None, type=float,
        help='ytick multiple')

    parser.add_argument('--xmaxn', default=4, type=int, help='xtick maxn')
    parser.add_argument('--ymaxn', default=4, type=int, help='ytick maxn')

    parser.add_argument('--fontsize' , default='large')
    parser.add_argument('--labelsize', default=None)
    parser.add_argument('--ticksize' , default=None)
    parser.add_argument('--titlesize', default=None)

    parser.add_argument('--legend', action='store_true', help='show legend')
    parser.add_argument('--legendsize', default='xx-small')
    parser.add_argument('--loc', default='best')

    parser.add_argument('--show', action='store_true', help='show plot')
    parser.add_argument('--save', action='store_true', help='save plot')

    opts = parser.parse_args(args)

    if opts.titlesize is None: opts.titlesize = opts.fontsize
    if opts.labelsize is None: opts.labelsize = opts.fontsize
    if opts.ticksize  is None: opts.ticksize  = opts.fontsize

    if opts.xmin: opts.xlim[0] = opts.xmin
    if opts.xmax: opts.xlim[1] = opts.xmax

    if opts.ymin: opts.ylim[0] = opts.ymin
    if opts.ymax: opts.ylim[1] = opts.ymax

    opts.xmin, opts.xmax = opts.xlim
    opts.ymin, opts.ymax = opts.ylim

    if opts.all:
        opts.w1  = True
        opts.w2  = True
        opts.d12 = True
        opts.d21 = True
        opts.s12 = True

    popts = [opts.w1, opts.w2, opts.d12, opts.d21, opts.s12, opts.r1]
    if not any(popts):
        raise RuntimeError, 'must use --w1, --w2, etc to chose plot type'

    if not opts.show and not opts.save:
        opts.show = True

    return opts

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    data = lm.common.load_pkl(opts.ifile)

    image = data['image']
    r1    = data['r1'] * opts.rscale
    w1    = data['w1']
    w2    = data['w2']

    d12   = w2 - w1
    d21   = w1 - w2
    s12   = w1 + w2

    kwargs = dict(bins=opts.bins)

    counts_w1, edges = np.histogram(r1, weights=w1, **kwargs)
    counts_w2, edges = np.histogram(r1, weights=w2, **kwargs)
    counts_w3, edges = np.histogram(r1, weights=d21, **kwargs)
    counts_w4, edges = np.histogram(r1, weights=d12, **kwargs)
    counts_w5, edges = np.histogram(r1, weights=s12, **kwargs)

    counts_r1, edges = np.histogram(r1, **kwargs)

    counts_w1 = counts_w1 / counts_r1
    counts_w2 = counts_w2 / counts_r1
    counts_w3 = counts_w3 / counts_r1
    counts_w4 = counts_w4 / counts_r1
    counts_w5 = counts_w5 / counts_r1

    if opts.abs:
        counts_w1 = abs(counts_w1)
        counts_w2 = abs(counts_w2)
        counts_w3 = abs(counts_w3)
        counts_w4 = abs(counts_w4)
        counts_w5 = abs(counts_w5)

    if opts.normalize:
        counts_w1 = lm.surface.linear_mapping(counts_w1, 0, 1)
        counts_w2 = lm.surface.linear_mapping(counts_w2, 0, 1)
        counts_w3 = lm.surface.linear_mapping(counts_w3, 0, 1)
        counts_w4 = lm.surface.linear_mapping(counts_w4, 0, 1)
        counts_w5 = lm.surface.linear_mapping(counts_w5, 0, 1)

    if opts.detrend:
        counts_w1 = counts_w1 - np.average(counts_w1)
        counts_w2 = counts_w2 - np.average(counts_w2)
        counts_w3 = counts_w3 - np.average(counts_w3)
        counts_w4 = counts_w4 - np.average(counts_w4)
        counts_w5 = counts_w5 - np.average(counts_w5)

    edges = 0.5 * (edges[1:] + edges[:-1])

    fig, ax1 = lm.plot.subplots(1, 1, *opts.figure)

    lm.plot.title(opts.title, fontsize=opts.titlesize)
    plt.xlabel(opts.xlabel, size=opts.labelsize)
    plt.ylabel(opts.ylabel, size=opts.labelsize)
    plt.tick_params(labelsize=opts.ticksize)

    if opts.w1:
        plt.plot(edges, counts_w1, color=lm.plot.colors.r1, label='$(w1)$')
    if opts.w2:
        plt.plot(edges, counts_w2, color=lm.plot.colors.o1, label='$(w2)$')
    if opts.d21:
        plt.plot(edges, counts_w3, color=lm.plot.colors.y1, label='$(w1-w2)$')
    if opts.d12:
        plt.plot(edges, counts_w4, color=lm.plot.colors.g2, label='$(w2-w1)$')
    if opts.s12:
        plt.plot(edges, counts_w5, color=lm.plot.colors.b1, label='$(w1+w2)$')
    if opts.r1:
        plt.plot(edges, counts_r1, color='k', label='$(r1)$')

    if opts.legend:
        plt.legend(loc=opts.loc, fontsize=opts.legendsize, frameon=False)

    if opts.xmax is None:
        opts.xmax = np.amax(r1)

    if opts.xmin is None:
        opts.xmin = 0

    plt.xlim(opts.xmin, opts.xmax)
    plt.ylim(opts.ymin, opts.ymax)

    lm.plot.maxn_locator(x=opts.xmaxn)
    lm.plot.maxn_locator(y=opts.ymaxn)

    if opts.xmult: lm.plot.multiple_locator(x=opts.xmult)
    if opts.ymult: lm.plot.multiple_locator(y=opts.ymult)

    ax1.yaxis.get_offset_text().set_size(opts.ticksize)

    if opts.save:
        handle = lm.common.format_output(stub=opts.stub, name='rdf',
            ext=opts.ext)
        print 'saved: %s' % handle
        lm.plot.save(handle)

    if opts.show:
        plt.show()