# -*- coding: utf-8 -*-
"""
morph.py
========

.. argparse::
    :module: morph
    :func: create_parser
    :prog: morph.py

.. moduleauthor:: Adam Gagorik <adam.gagorik@gmail.com>
"""
import matplotlib.pyplot as plt
import langmuir as lm
import numpy as np
import argparse
import sys
import os

desc = """
Morph between two surfaces.
"""

def create_parser():
    parser = argparse.ArgumentParser()
    parser.description = desc

    parser.add_argument(dest='ifile1', type=str, metavar='input',
        help='input file 1')
    parser.add_argument(dest='ifile2', type=str, metavar='input',
        help='input file 2')

    parser.add_argument('--stub', default='', type=str, metavar='stub',
        help='output file stub')

    parser.add_argument('--format', default='{stub}_{name}_{i}.{ext}',
        type=str, help='format of file names: use {stub},{i},{x},{y},{ext}')

    parser.add_argument('--func', default='x', type=str, metavar='str',
        help='interpolating polynomial as a string, for example: np.sin(x)')

    parser.add_argument('--xvalues', default='np.linspace(0, 1, 10)',
        type=str, nargs='*', metavar='str',
        help='x-values as a string, for example: np.linspace(0, 1, 10)')

    parser.add_argument('--nolmap', action='store_false',
        help='do not map yvalues to [0,1]')

    parser.add_argument('--vtk', action='store_true', help='save vtk files ' +
        'for paraview animation')

    parser.add_argument('--show', action='store_true', help='show mpl plot')
    parser.add_argument('--save', action='store_true', help='save mpl plot')

    return parser

def get_arguments(args=None):
    parser = create_parser()
    opts = parser.parse_args(args)

    if not os.path.exists(opts.ifile1):
        parser.print_help()
        print >> sys.stderr, '\nfile does not exist: %s' % opts.ifile1
        sys.exit(-1)

    if not os.path.exists(opts.ifile2):
        parser.print_help()
        print >> sys.stderr, '\nfile does not exist: %s' % opts.ifile2
        sys.exit(-1)

    if not opts.save and not opts.show:
        opts.show = True

    return opts

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    surf1 = lm.surface.load(opts.ifile1)
    surf2 = lm.surface.load(opts.ifile2)

    grid1 = lm.grid.Grid(*surf1.shape)
    grid2 = lm.grid.Grid(*surf2.shape)

    print grid1
    print grid2

    assert grid1.shape == grid2.shape

    opts.xvalues = np.asanyarray(eval(opts.xvalues))

    poly = lambda x : eval(opts.func)
    opts.yvalues = poly(opts.xvalues)

    if opts.nolmap:
        pass
    else:
        opts.xvalues = lm.surface.linear_mapping(opts.xvalues)
        opts.yvalues = lm.surface.linear_mapping(opts.yvalues)

    for i, (x, y) in enumerate(zip(opts.xvalues, opts.yvalues)):
        handle = lm.common.format_output(opts.format,
            stub=opts.stub, name='morph', ext='pkl', i=i, x=x, y=y)
        s = x * surf2 + (1 - x) * surf1
        lm.surface.save(handle, s)
        print 'x = %.3f, f = %.3f saved: %s' % (x, y, handle)

        if opts.vtk:
            handle = lm.common.format_output(opts.format, stub=opts.stub,
                name='morph', ext='vti', i=i, x=x, y=y)
            lm.surface.save_vti(handle, s, origin=grid1.origin,
                spacing=grid1.spacing, name='surface')

    if opts.show or opts.save:
        fig, ax1 = lm.plot.subplots(1, 1, t=1)
        plt.axhline(np.amin(opts.yvalues), lw=1.5, color='k', ls=':')
        plt.axhline(np.amax(opts.yvalues), lw=1.5, color='k', ls=':')
        plt.plot(opts.xvalues, opts.yvalues, 'bo-', label='surface 1',
                 mfc='w', mec='b', mew=1.5)
        plt.plot(opts.xvalues, 1.0 - opts.yvalues, 'ro-', label='surface 2',
                 mfc='w', mec='r', mew=1.5)
        plt.legend(loc='lower center', ncol=2, bbox_to_anchor=(0.5, 1.0),
            bbox_transform=ax1.transAxes)
        lm.plot.zoom(l=0, r=0)
        plt.xlabel('x')
        plt.ylabel('y')

    if opts.save:
        handle = lm.common.format_output(stub=opts.stub, name='interp',
            ext='pdf')
        lm.plot.save(handle)
        print 'saved: %s' % handle

    if opts.show:
        plt.show()