# -*- coding: utf-8 -*-
"""
equil.py
========

.. argparse::
    :module: equil
    :func: create_parser
    :prog: equil

.. moduleauthor:: Adam Gagorik <adam.gagorik@gmail.com>
"""
import matplotlib.pyplot as plt
import matplotlib as mpl
import langmuir as lm
import pandas as pd
import numpy as np
import argparse
import os

desc = """
Check simulation(s) for equilibration.
"""

def create_parser():
    parser = argparse.ArgumentParser()
    parser.description = desc

    parser.add_argument(dest='pkls', default=[], type=str, nargs='*',
        metavar='pkl', help='input files')

    parser.add_argument('-r', action='store_true',
        help='search for files recursivly')
    
    parser.add_argument('--ycol', default='drain:current', type=str,
                        help='ycolumn to plot')    
    
    parser.add_argument('--legend', action='store_true', help='show legend')
    parser.add_argument('--zoom', default=0.05, type=float, help='zoom factor')

    parser.add_argument('--show', action='store_true', help='show plot')
    parser.add_argument('--save', action='store_true', help='save plot')

    return parser

def get_arguments(args=None):
    parser = create_parser()
    opts = parser.parse_args(args)

    if not opts.pkls:
        opts.pkls = [os.getcwd()]

    pkls = []
    for pkl in opts.pkls:
        if os.path.isdir(pkl):
            pkls.extend(lm.find.pkls(pkl, stub='combined*', r=opts.r))
        else:
            pkls.append(pkl)
    opts.pkls = pkls

    if not opts.pkls:
        raise RuntimeError, 'can not find any pkl files'

    if not opts.show and not opts.save:
        opts.show = True

    return opts

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    y_col = opts.ycol

    cmap = mpl.cm.get_cmap('spectral')
    colors = [cmap(val) for val in np.linspace(0, 1, len(opts.pkls))]

    fig, ax1 = lm.plot.subplots(1, 1, b=0.75, l=1.5, r=1.5)
    xmax = 1
    ymax = 0

    handles, labels = [], []
    for i, pkl in enumerate(opts.pkls):
        print i, pkl
        data = lm.common.load_pkl(pkl)
        data = lm.analyze.calculate(data)
        data[y_col].plot(lw=0.25, color='k')
        roll = int(len(data.index) / float(32))
        pd.rolling_mean(data[y_col], roll).plot(lw=1, color=colors[i])
        handles.append(plt.Rectangle((0, 0), 1, 1, fc=colors[i], lw=0))
        labels.append('sim%d' % (i + 1))
        xmax = max(xmax, data.index[-1])
        ymax = max(ymax, data.ix[int(data.index.size * 0.10):, y_col].max())
    ymax = abs(ymax)

    if opts.legend:
        plt.legend(handles, labels, prop=dict(size='xx-small'),
                   loc='upper left', bbox_transform=ax1.transAxes,
                   bbox_to_anchor=(1, 1))

    plt.ylim(-ymax, ymax)
    plt.xlim(0, xmax)
    lm.plot.zoom(l=0, r=0, factor=opts.zoom)
    plt.ticklabel_format(scilimits=(-4, 4))
    plt.tick_params(labelsize='small')
    lm.plot.maxn_locator(x=5, y=5)
    plt.xlabel('time (ps)', size='small')
    plt.ylabel(y_col, size='small')

    if opts.save:
        handle = lm.common.format_output(stub=opts.stub, name='rdf',
            ext=opts.ext)
        print 'saved: %s' % handle
        lm.plot.save(handle)

    if opts.show:
        plt.show()