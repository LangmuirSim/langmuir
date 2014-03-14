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

    parser.add_argument(dest='ifile', default='conv.pkl', type=str, nargs='?',
                        metavar='input', help='input file')

    parser.add_argument('--stub', default='', type=str, metavar='stub',
                        help='output file stub', required=True)
    parser.add_argument('--ext', default='pdf', type=str, metavar='str',
        choices=['png', 'pdf', 'jpg'], help='output file type')

    parser.add_argument('--figure', default=(6.0, 6.0, 1.5, 1.0, 1.0, 1.5),
        help='figure dimensions (w, h, l, r, t, b)')

    parser.add_argument('--c0', action='store_true', help='x')
    parser.add_argument('--c1', action='store_true', help='y')
    parser.add_argument('--c2', action='store_true', help='corr')
    parser.add_argument('--c3', action='store_true', help='conv')
    parser.add_argument('--all', action='store_true', help='all')

    parser.add_argument('--title'   , default='')
    parser.add_argument('--xlabel'  , default='$\Delta r$')
    parser.add_argument('--ylabel'  , default='')

    parser.add_argument('--xlim', default=(0, None), nargs=2, type=float)
    parser.add_argument('--xmax', default=None, type=float)
    parser.add_argument('--xmin', default=None, type=float)

    parser.add_argument('--ylim', default=(-1.1, 1.1), nargs=2, type=float)
    parser.add_argument('--ymax', default=None, type=float)
    parser.add_argument('--ymin', default=None, type=float)

    parser.add_argument('--xmult', default=None, type=float,
        help='xtick multiple')
    parser.add_argument('--ymult', default=None, type=float,
        help='ytick multiple')

    parser.add_argument('--xmaxn', default=6, type=int, help='xtick maxn')
    parser.add_argument('--ymaxn', default=6, type=int, help='ytick maxn')

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
        opts.c0 = True
        opts.c1 = True
        opts.c2 = True
        opts.c3 = True

    popts = [opts.c0, opts.c1, opts.c2, opts.c3]
    if not any(popts):
        raise RuntimeError, 'must use --c0, --c1, etc to chose plot type'

    if not opts.show and not opts.save:
        opts.show = True

    return opts

def calculate_bins(array):
    bwidth = 1.0
    brange = [0, int(np.amax(array))]
    if brange[1] % 2 == 0:
        pass
    else:
        brange[1] += 1
    bins = int(brange[1] / bwidth)
    return bwidth, brange, bins

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    data = lm.common.load_pkl(opts.ifile)
    corr = data['corr']
    conv = data['conv']
    x, y = data['x'], data['y']

    grid = lm.grid.Grid(*x.shape)
    mesh = lm.grid.PrecalculatedMesh(grid)

    bwidth, brange, bins = calculate_bins(mesh.r1)

    kwargs = dict(bins=bins, range=brange)

    counts_c0, edges = np.histogram(mesh.r1.flat, weights=x.flat, **kwargs)
    counts_c1, edges = np.histogram(mesh.r1.flat, weights=y.flat, **kwargs)
    counts_c2, edges = np.histogram(mesh.r1.flat, weights=corr.flat, **kwargs)
    counts_c3, edges = np.histogram(mesh.r1.flat, weights=conv.flat, **kwargs)

    edges = 0.5*(edges[1:] + edges[:-1])

    counts_c0 = lm.surface.linear_mapping(counts_c0, -1, 1)
    counts_c1 = lm.surface.linear_mapping(counts_c1, -1, 1)
    counts_c2 = lm.surface.linear_mapping(counts_c2, -1, 1)
    counts_c3 = lm.surface.linear_mapping(counts_c3, -1, 1)

    fig, ax1 = lm.plot.subplots(1, 1, *opts.figure)

    lm.plot.title(opts.title, fontsize=opts.titlesize)
    plt.xlabel(opts.xlabel, size=opts.labelsize)
    plt.ylabel(opts.ylabel, size=opts.labelsize)
    plt.tick_params(labelsize=opts.ticksize)

    if opts.c0:
        plt.plot(edges, counts_c0, color=lm.plot.colors.r1, lw=2, label='x')
    if opts.c1:
        plt.plot(edges, counts_c1, color=lm.plot.colors.b1, lw=2, label='y')
    if opts.c2:
        plt.plot(edges, counts_c2, color=lm.plot.colors.g2, lw=2, label='corr')
    if opts.c3:
        plt.plot(edges, counts_c3, color=lm.plot.colors.o1, lw=2, label='conv')

    if opts.xmax is None:
        opts.xmax = np.amax(mesh.r1.flat)

    opts.xlim = (opts.xmin, opts.xmax)

    plt.ylim(*opts.ylim)
    plt.xlim(*opts.xlim)

    lm.plot.maxn_locator(x=opts.xmaxn)
    lm.plot.maxn_locator(y=opts.ymaxn)

    if opts.xmult: lm.plot.multiple_locator(x=opts.xmult)
    if opts.ymult: lm.plot.multiple_locator(y=opts.ymult)

    ax1.xaxis.get_offset_text().set_size(opts.ticksize)
    ax1.yaxis.get_offset_text().set_size(opts.ticksize)

    if opts.legend:
        plt.legend(loc=opts.loc, fontsize=opts.legendsize, frameon=False)

    if opts.save:
        handle = lm.common.format_output(stub=opts.stub, name='',
            ext=opts.ext)
        print 'saved: %s' % handle
        lm.plot.save(handle)

    if opts.show:
        plt.show()