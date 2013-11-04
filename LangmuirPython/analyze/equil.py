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
    desc = "make plots for drain currents vs time"
    parser = argparse.ArgumentParser(description=desc)
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

def plot1(ycol='drain:current', title='Drain Current', oname='equil1.pdf',
          ymin=None, ymax=None):
    fig, ax1 = plt.subplots(1, 1, figsize=(12, 8))

    cmap = mpl.cm.get_cmap('gist_rainbow')
    colors = [cmap(v) for v in np.linspace(0, 1, len(opts.ifiles))]

    handles, labels = [], []
    for i, pkl in enumerate(opts.ifiles):
        print '(%2d/%2d): %s ' % (i, len(opts.ifiles),
            os.path.relpath(pkl, work))
        pkl = lm.analyze.load_pkl(pkl)
        pkl = lm.analyze.calculate(pkl)

        pkl['simulation:time'] *= cfactor('ps', opts.tunit)
        pkl['simulation:time'].apply(lambda x : int(x))
        pkl = pkl.set_index('simulation:time')
        roll = int(len(pkl.index) / float(32))

        plt.sca(ax1)
        label = 'sim%d' % (i + 1)
        pkl[ycol].plot(lw=0.25, color='k', label=label)
        pd.rolling_mean(pkl[ycol], roll).plot(lw=1, color=colors[i])

        labels.append(label)
        handles.append(plt.Rectangle((0, 0), 1, 1, fc=colors[i], lw=0))

    plt.locator_params(nbins=6)
    plt.ticklabel_format(scilimits=(-4, 4))
    plt.xlim(0, pkl.index[-1])

    plt.ylim(ymin, ymax)

    plt.legend(handles, labels, prop=dict(size='small'), loc='upper left',
        bbox_transform=ax1.transAxes, bbox_to_anchor=(1, 1))

    plt.xlabel('step %s' % opts.tunit.dimensionality.latex)
    plt.ylabel('nA')
    plt.title(title)

    plt.tight_layout()
    plt.subplots_adjust(right=0.85)

    if opts.save:
        plt.savefig(oname)

def plot2(ycol='drain:current', title='Drain Current', oname='equil2.pdf',
          ymin=None, ymax=None):
    fig, ax1 = plt.subplots(1, 1, figsize=(12, 8))

    cmap = mpl.cm.get_cmap('gist_rainbow')
    colors = [cmap(v) for v in np.linspace(0, 1, len(opts.ifiles))]

    handles, labels = [], []

    for i, pkl in enumerate(opts.ifiles):
        print '(%2d/%2d): %s ' % (i, len(opts.ifiles),
            os.path.relpath(pkl, work))
        pkl = lm.analyze.load_pkl(pkl)
        pkl = lm.analyze.calculate(pkl)

        delta = pkl.ix[pkl.index[-1]] - pkl
        delta = lm.analyze.calculate(delta)
        roll = int(len(delta.index) / float(32))

        label = 'sim%d' % (i + 1)
        delta[ycol].plot(lw=1, color=colors[i], label=label)
        pd.rolling_mean(delta[ycol], roll).plot(lw=1, color='k')

        labels.append(label)
        handles.append(plt.Rectangle((0, 0), 1, 1, fc=colors[i], lw=0))

    plt.locator_params(nbins=6)
    plt.ticklabel_format(scilimits=(-4, 4))
    plt.xlim(0, pkl.index[-1])
    plt.ylim(ymin, ymax)
    plt.legend(handles, labels, prop=dict(size='small'), loc='upper left',
        bbox_transform=ax1.transAxes, bbox_to_anchor=(1, 1))

    plt.xlabel('Equilibration %s' % opts.tunit.dimensionality.latex)
    plt.ylabel('nA')
    plt.title(title)

    plt.tight_layout()
    plt.subplots_adjust(right=0.85)

    if opts.save:
        plt.savefig(oname)

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    plot1('drain:current', 'Drain', 'equil.pdf', -0.5, 0.5)

    if opts.show:
        plt.show()
