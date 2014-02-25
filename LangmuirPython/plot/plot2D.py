# -*- coding: utf-8 -*-
"""
@author: adam
"""
import matplotlib.pyplot as plt
import matplotlib as mpl
import langmuir as lm
import numpy as np
import argparse
import textwrap
import sys
import os

desc = """
Plot contour surface.
"""

def get_arguments(args=None):
    parser = argparse.ArgumentParser()
    parser.description = desc

    parser.add_argument(dest='ifile', default='image.pkl', type=str, nargs='?',
        metavar='input', help='input file')
    parser.add_argument(dest='ofile', default='image.png', type=str, nargs='?',
        metavar='plot.png', help='output file name')

    parser.add_argument('--plane', default='xy', type=str, metavar='str',
        choices=['xy', 'yz', 'xz'], help='plotting plane')
    parser.add_argument('--level', default=0, type=int, metavar='int',
        help='index of plane')

    parser.add_argument('--figure', default=(6.0, 6.0, 1.5, 1.0, 1.0, 1.5),
        help='figure dimensions (w, h, l, r, t, b)')

    parser.add_argument('--title'   , default='')
    parser.add_argument('--units', default=None, type=str, metavar='str',
        help='units of xy axis')

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

    parser.add_argument('--xmaxn', default=6, type=int, help='xtick maxn')
    parser.add_argument('--ymaxn', default=6, type=int, help='ytick maxn')

    parser.add_argument('--fontsize' , default='large')
    parser.add_argument('--labelsize', default=None)
    parser.add_argument('--ticksize' , default=None)
    parser.add_argument('--titlesize', default=None)
    
    parser.add_argument('--colorbar', action='store_true',
        help='show colorbar')
    parser.add_argument('--clabel', default='', type=str, metavar='str',
        help='colorbar label')

    parser.add_argument('--clevels', default=32, type=int, metavar='int',
        help='contour levels')
    parser.add_argument('--clines', default=0, type=int, metavar='int',
        help='contour lines')
    parser.add_argument('--cmap', default=['spectral'], type=str, nargs='*',
        metavar='str', help='color map name, number, or list of colors')

    parser.add_argument('--grid', action='store_true', help='show grid')
    parser.add_argument('--show', action='store_true', help='show plot')
    parser.add_argument('--save', action='store_true', help='save plot')

    parser.add_argument('--threshold', action='store_true',
        help='apply threshold to image')
    parser.add_argument('--tvalue', default=0.0, type=float, metavar='float',
        help='thresholding value')
    parser.add_argument('--lmap', action='store_true',
        help='map image values to [0,1]')
    parser.add_argument('--dots', action='store_true',
        help='scatter dots using threshold value')
    parser.add_argument('--above', action='store_true',
        help='find dots above --tvalue')

    opts = parser.parse_args(args)

    if not opts.show and not opts.ofile:
        parser.print_help()
        print >> sys.stderr, '\nnothing to do, use --show or --stub'

    if opts.clabel:
        opts.colorbar = True

    if opts.titlesize is None: opts.titlesize = opts.fontsize
    if opts.labelsize is None: opts.labelsize = opts.fontsize
    if opts.ticksize  is None: opts.ticksize  = opts.fontsize

    if opts.xmin: opts.xlim[0] = opts.xmin
    if opts.xmax: opts.xlim[1] = opts.xmax

    if opts.ymin: opts.ylim[0] = opts.ymin
    if opts.ymax: opts.ylim[1] = opts.ymax

    opts.xmin, opts.xmax = opts.xlim
    opts.ymin, opts.ymax = opts.ylim

    if len(opts.cmap) > 1:
        opts.cmap = mpl.colors.ListedColormap(opts.cmap)
    else:
        opts.cmap = opts.cmap[0]
        cmaps = sorted([k for k in mpl.cm.datad.keys() if not '_' in k])
        if not opts.cmap in cmaps:
            try:
                opts.cmap = cmaps[int(opts.cmap)]
            except ValueError:
                parser.print_help()
                print >> sys.stderr, '\ninvalid colormap: %s\n' % opts.cmap
                print >> sys.stderr, \
                    '\n'.join(textwrap.wrap(', '.join(cmaps), width=60))
                sys.exit(-1)
        opts.cmap = mpl.cm.get_cmap(opts.cmap)

    if opts.dots and not opts.lmap and opts.tvalue == 0.5:
        print >> sys.stderr, 'warning: using --dots with --tvalue 0.5 ' + \
            'without linear mapping'

    if not opts.show and not opts.save:
        opts.show = True

    return opts

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    image = lm.surface.load(opts.ifile)
    grid = lm.grid.Grid(*image.shape)

    if opts.lmap:
        image = lm.surface.linear_mapping(image)

    if opts.threshold:
        image = lm.surface.threshold(image, v=opts.tvalue)

    if opts.plane == 'xy':
        xlabel, ylabel = 'x', 'y'
        x, y = grid.mx, grid.my
        s = np.index_exp[:,:,opts.level]
        xmin, xmax = 0, grid.lx
        ymin, ymax = 0, grid.ly
        if grid.nx <= 1 or grid.ny <= 1:
            print >> sys.stderr, 'no data in plane xy!'
            print >> sys.stderr, 'data.shape = %s' % list(image.shape)
            sys.exit(-1)

    if opts.plane == 'yz':
        xlabel, ylabel = 'y', 'z'
        x, y = grid.my, grid.mz
        s = np.index_exp[opts.level,:,:]
        xmin, xmax = 0, grid.ly
        ymin, ymax = 0, grid.lz
        if grid.ny <= 1 or grid.nz <= 1:
            print >> sys.stderr, 'no data in plane yz!'
            print >> sys.stderr, 'data.shape = %s' % list(image.shape)
            sys.exit(-1)

    if opts.plane == 'xz':
        xlabel, ylabel = 'x', 'z'
        x, y = grid.mx, grid.mz
        s = np.index_exp[:,opts.level,:]
        xmin, xmax = 0, grid.lx
        ymin, ymax = 0, grid.lz
        if grid.nx <= 1 or grid.nz <= 1:
            print >> sys.stderr, 'no data in plane xz!'
            print >> sys.stderr, 'data.shape = %s' % list(image.shape)
            sys.exit(-1)

    if opts.units:
        xlabel = xlabel + ' (%s)' % opts.units
        ylabel = ylabel + ' (%s)' % opts.units

    fig, ax1 = lm.plot.subplots(1, 1, *opts.figure)
    smap = plt.contourf(x[s], y[s], image[s], opts.clevels, cmap=opts.cmap)
    if opts.clines:
        plt.contour(x[s], y[s], image[s], opts.clines, colors='k',
            linestyles='-', linewidths=1)

    if opts.dots:
        if opts.above:
            xdots, ydots = x[image > opts.tvalue], y[image > opts.tvalue]
        else:
            xdots, ydots = x[image < opts.tvalue], y[image < opts.tvalue]
        if not len(xdots):
            print >> sys.stderr, 'warning: found no dots, --tvalue wrong?'
        else:
            plt.scatter(xdots, ydots, marker='.', s=1)

    lm.plot.title(opts.title, fontsize=opts.titlesize)

    plt.xlabel(xlabel, fontsize=opts.labelsize)
    plt.ylabel(ylabel, fontsize=opts.labelsize)

    plt.tick_params(labelsize=opts.ticksize)
    plt.xlim(xmin, xmax)
    plt.ylim(ymin, ymax)

    if opts.colorbar:
        cax = lm.plot.create_colorbar_axes(width=0.03)
        cbar = plt.colorbar(smap, cax)
        cax.tick_params(labelsize=opts.ticksize)
        cax.yaxis.get_offset_text().set_size(opts.ticksize)
        cbar.formatter.set_powerlimits((-3, 3))
        cbar.set_label(opts.clabel, fontsize=opts.labelsize)
        cbar.update_ticks()
        lm.plot.fix_colorbar_labels(cax)


    lm.plot.maxn_locator(x=opts.xmaxn)
    lm.plot.maxn_locator(y=opts.ymaxn)

    if opts.xmult: lm.plot.multiple_locator(x=opts.xmult)
    if opts.ymult: lm.plot.multiple_locator(y=opts.ymult)

    ax1.xaxis.get_offset_text().set_size(opts.ticksize)
    ax1.yaxis.get_offset_text().set_size(opts.ticksize)

    ax1.xaxis.set_minor_locator(mpl.ticker.AutoMinorLocator(2))
    ax1.yaxis.set_minor_locator(mpl.ticker.AutoMinorLocator(2))

    if opts.grid:
        plt.grid(which='major', lw=0.5, ls='-')
        plt.grid(which='minor', lw=0.5, ls=':')

    if opts.save:
        print 'saved: %s' % opts.ofile
        lm.plot.save(opts.ofile)

    if opts.show:
        plt.show()