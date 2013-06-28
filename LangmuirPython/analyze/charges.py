# -*- coding: utf-8 -*-
import matplotlib.pyplot as plt
import quantities as units
import matplotlib as mpl
import langmuir as lm
import pandas as pd
import numpy as np
import argparse
import os

def get_arguments(args=None):
    parser = argparse.ArgumentParser()
    parser.add_argument(dest='ifiles', nargs='*', default=[], type=str,
               help='input file name')
    parser.add_argument('--tunit', type=str, default='ns',
               help='unit for time axis')
    parser.add_argument('--show', default=False, action='store_true',
               help='open mpl window')
    parser.add_argument('--save', default=False, action='store_true',
               help='save plot')
    opts = parser.parse_args(args)
    opts.work = os.getcwd()

    if not opts.ifiles:
        opts.ifiles = lm.find.pkls(work, 'combined*', 1, 1, 1)

    if not opts.save:
        opts.show = True

    opts.tunit = units.CompoundUnit(opts.tunit)

    return opts

def cfactor(u1, u2):
    if isinstance(u1, str):
        u1 = units.CompoundUnit(u1)
    if isinstance(u2, str):
        u2 = units.CompoundUnit(u2)
    return float(u1.rescale(u2))

def plot1():
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 8), sharey=True,
          sharex=True)

    cmap = mpl.cm.get_cmap('gist_rainbow')
    colors = [cmap(v) for v in np.linspace(0, 1, len(opts.ifiles))]

    handles, labels = [], []

    for i, pkl in enumerate(opts.ifiles):
        print '(%2d/%2d): %s ' % (i, len(opts.ifiles),
            os.path.relpath(pkl, work))
        pkl = lm.analyze.load_pkl(pkl)

        pkl['simulation:time'] *= cfactor('ps', opts.tunit)
        pkl['simulation:time'].apply(lambda x : int(x))
        pkl = pkl.set_index('simulation:time')
        roll = int(len(pkl.index) / float(32))

        plt.sca(ax1)
        pkl['electron:count'].plot(lw=0.25, color=colors[i],
            label='sim%d' % (i + 1))
        pd.rolling_mean(pkl['electron:count'], roll).plot(
            lw=1.0, color='k')

        plt.sca(ax2)
        pkl['hole:count'].plot(lw=0.25, color=colors[i],
            label='sim%d' % (i + 1))
        pd.rolling_mean(pkl['hole:count'], roll).plot(
            lw=1.0, color='k')

        labels.append('sim%d' % (i + 1))
        handles.append(plt.Rectangle((0, 0), 1, 1, fc=colors[i], lw=0))

    plt.locator_params(nbins=6)
    plt.ticklabel_format(scilimits=(-4, 4))
    plt.xlim(0, pkl.index[-1])
    plt.legend(handles, labels, prop=dict(size='small'), loc='upper left',
        bbox_transform=ax2.transAxes, bbox_to_anchor=(1, 1))

    plt.sca(ax1)
    plt.xlabel('step %s' % opts.tunit.dimensionality.latex)
    plt.title('electrons')

    plt.sca(ax2)
    plt.xlabel('step %s' % opts.tunit.dimensionality.latex)
    plt.title('holes')

    plt.tight_layout()
    plt.subplots_adjust(right=0.85)

    if opts.save:
        plt.savefig('charges.pdf')

def plot2(bmin=0, bmax=None, bins=100):
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 8), sharey=True,
          sharex=True)

    cmap = mpl.cm.get_cmap('gist_rainbow')
    colors = [cmap(v) for v in np.linspace(0, 1, len(opts.ifiles))]

    if bmax is None:
        emax = 0
        hmax = 0
        for i, pkl in enumerate(opts.ifiles):
            print '(%2d/%2d): %s ' % (i, len(opts.ifiles),
                os.path.relpath(pkl, work))
            pkl = lm.analyze.load_pkl(pkl)

            emax = max(pkl['electron:count'].max(), emax)
            hmax = max(pkl['hole:count'].max(), hmax)
        print 'emax:', emax
        print 'hmax:', hmax

        bmax = max(emax, hmax)
        bmax = int(np.ceil(bmax / 10.0) * 10.0)

        print 'bmax:', bmax

    for i, pkl in enumerate(opts.ifiles):
        print '(%2d/%2d): %s ' % (i, len(opts.ifiles),
            os.path.relpath(pkl, work))
        pkl = lm.analyze.load_pkl(pkl)

        pkl['simulation:time'] *= cfactor('ps', opts.tunit)
        pkl['simulation:time'].apply(lambda x : int(x))
        pkl = pkl.set_index('simulation:time')

        plt.sca(ax1)
        pkl['electron:count'].hist(color=colors[i], bins=bins,
            range=(bmin, bmax), alpha=0.75, lw=0, histtype='stepfilled',
            label='sim%d' % (i + 1))
        pkl['electron:count'].hist(color='k', bins=bins, range=(bmin, bmax),
            alpha=1, lw=1, histtype='step')

        plt.sca(ax2)
        pkl['hole:count'].hist(color=colors[i], bins=bins,
            range=(bmin, bmax), alpha=0.75, lw=0, histtype='stepfilled',
            label='sim%d' % (i + 1))
        pkl['hole:count'].hist(color='k', bins=bins, range=(bmin, bmax),
            alpha=1, lw=1, histtype='step')

    plt.sca(ax1)
    plt.locator_params(nbins=5)
    plt.ticklabel_format(scilimits=(-4, 4))
    plt.legend(prop=dict(size='small'), bbox_transform=ax2.transAxes,
               bbox_to_anchor=(1, 1), loc='upper left')
    plt.xlim(bmin, bmax)
    lm.plot.zoom(t=0, b=0)

    plt.sca(ax1)
    plt.xlabel('count')
    plt.title('electrons')
    plt.grid()

    plt.sca(ax2)
    plt.xlabel('count')
    plt.title('holes')
    plt.grid()

    plt.tight_layout()
    plt.subplots_adjust(right=0.85)

    if opts.save:
        plt.savefig('charges_hist.pdf')

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    plot1()
    plot2()

    if opts.show:
        plt.show()