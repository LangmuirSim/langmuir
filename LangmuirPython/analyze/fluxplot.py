# -*- coding: utf-8 -*-
"""
Created on Fri May 10 08:07:05 2013

@author: adam
"""
import matplotlib.pyplot as plt
import matplotlib as mpl
import numpy as np
import langmuir
import argparse
import os
import re

def get_arguments(args=None):
    parser = argparse.ArgumentParser()
    parser.add_argument(dest='ifile', nargs='?', default=None, type=str,
               help='input file name')
    parser.add_argument('--work', default=os.getcwd(), type=str,
               help='working directory')
    parser.add_argument('--flux', default='xDrain', type=str,
               help='flux agent')
    parser.add_argument('--nology', default=False, action='store_true',
               help='use log scale for y')
    parser.add_argument('--mode', default='all', type=str, choices=['all',
               'rate', 'prob', 'flux', 'dflux'], help='plot type')
    parser.add_argument('--show', default=False, action='store_true',
               help='open mpl window')
    parser.add_argument('--save', default=False, action='store_true',
               help='save plot')
    opts = parser.parse_args(args)
    if not opts.ifile:
        try:
            opts.ifile = langmuir.find.pkl(opts.work, stub='combined*',
                    at_least_one=False)
        except RuntimeError:
            opts.ifile = langmuir.find.dat(opts.work, at_least_one=True)
    opts.ifile = os.path.relpath(opts.ifile, os.getcwd())
    opts.work = os.path.relpath(opts.work, os.getcwd())
    if not opts.save:
        opts.show = True
    return opts

def mathtt(s):
    return re.sub(r'\s', r'\,', r'$\mathtt{%s}$' % s)

def plot_rate(data):
    fig, ax1 = langmuir.plot.subplots(t=1)
    x = data['simulation:time']
    y = data['%s:rate' % opts.flux]
    plt.plot(x, y, 'r-', lw=2)
    fit = langmuir.fit.FitLinear(x[x.size/2:], y[y.size/2:])
    fit.plot(x.min(), x.max(), zorder=-10, label=fit.elabel)
    plt.legend(loc='lower right', prop=dict(size='small'), borderaxespad=0,
               bbox_transform=ax1.transAxes, bbox_to_anchor=(1, 1))
    if not opts.nology: plt.yscale('log')
    plt.xlim(x.min(), x.max())
    plt.xlabel(mathtt('time (ps)'))
    plt.ylabel(mathtt('rate (ps^{-1})'))
    plt.grid()
    plt.ticklabel_format(axis='x', scilimits=(-3, 3))
    if opts.save: langmuir.plot.save('%s-rate.pdf' % opts.flux.lower())

def plot_prob(data):
    fig, ax1 = langmuir.plot.subplots(t=1)
    x = data['simulation:time']
    y = data['%s:prob' % opts.flux]
    plt.plot(x, y, 'b-', lw=2)
    fit = langmuir.fit.FitLinear(x[x.size/2:], y[y.size/2:])
    fit.plot(x.min(), x.max(), zorder=-10, label=fit.elabel)
    plt.legend(loc='lower right', prop=dict(size='small'), borderaxespad=0,
               bbox_transform=ax1.transAxes, bbox_to_anchor=(1, 1))
    if not opts.nology: plt.yscale('log')
    plt.xlim(x.min(), x.max())
    plt.xlabel(mathtt('time (ps)'))
    plt.ylabel(mathtt('prob (\%)'))
    plt.grid()
    plt.ticklabel_format(axis='x', scilimits=(-3, 3))
    if opts.save: langmuir.plot.save('%s-prob.pdf' % opts.flux.lower())

def plot_flux(data):
    fig, ax1 = langmuir.plot.subplots(t=1)
    x = data['simulation:time']
    y1 = data['%s:attempt' % opts.flux]
    y2 = data['%s:success' % opts.flux]
    plt.plot(x, y1, 'r-', lw=2, label='A')
    plt.plot(x, y2, 'b-', lw=2, label='S')
    if not opts.nology: plt.yscale('log')
    plt.xlim(x.min(), x.max())
    plt.xlabel(mathtt('time (ps)'))
    plt.ylabel(mathtt('count (\#)'))
    plt.grid()
    plt.legend(prop=dict(size='small'), borderaxespad=0, bbox_to_anchor=(1, 1),
               bbox_transform=ax1.transAxes, loc='lower right', ncol=2)
    plt.ticklabel_format(axis='x', scilimits=(-3, 3))
    if opts.save: langmuir.plot.save('%s-flux.pdf' % opts.flux.lower())

def plot_dflux(data):
    fig, ax1 = langmuir.plot.subplots(t=1)
    x = data['simulation:time']
    y1 = data['%s:attempt' % opts.flux].diff()
    y2 = data['%s:success' % opts.flux].diff()
    plt.plot(x, y1, 'r-', lw=2, label='A')
    plt.plot(x, y2, 'b-', lw=2, label='S')
    if not opts.nology: plt.yscale('log')
    plt.xlim(x.min(), x.max())
    plt.xlabel(mathtt('time (ps)'))
    plt.ylabel(mathtt('count (\#)'))
    plt.grid()
    plt.legend(prop=dict(size='small'), borderaxespad=0, bbox_to_anchor=(1, 1),
               bbox_transform=ax1.transAxes, loc='lower right', ncol=2)
    plt.ticklabel_format(axis='x', scilimits=(-3, 3))
    if opts.save: langmuir.plot.save('%s-dflux.pdf' % opts.flux.lower())

if __name__ == '__main__':
    opts = get_arguments()

    if 'pkl' in opts.ifile:
        data = langmuir.analyze.load_pkl(opts.ifile)
    else:
        data = langmuir.analyze.load_dat(opts.ifile)

    data = langmuir.analyze.calculate(data)

    if opts.mode == 'all':
        plot_rate(data)
        plot_prob(data)
        plot_flux(data)
        plot_dflux(data)
    elif opts.mode == 'rate':
        plot_rate(data)
    elif opts.mode == 'prob':
        plot_prob(data)
    elif opts.mode == 'flux':
        plot_flux(data)
    elif opts.mode == 'dflux':
        plot_dflux(data)
    else:
        raise RuntimeError('unknown mode: %s' % opts.mode)

    if opts.show:
        plt.show()
