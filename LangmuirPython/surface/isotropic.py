# -*- coding: utf-8 -*-
"""
isotropic.py
============

.. argparse::
    :module: isotropic
    :func: create_parser
    :prog: isotropic.py

.. moduleauthor:: Adam Gagorik <adam.gagorik@gmail.com>
"""
import langmuir as lm
import numpy as np
import argparse
import sys
import os

desc = """
Create surface using convolution.
"""

def create_parser():
    parser = argparse.ArgumentParser()
    parser.description = desc

    parser.add_argument(dest='xsize', type=int, metavar='grid.x',
                        help='grid.x')
    parser.add_argument(dest='ysize', type=int, metavar='grid.y',
                        help='grid.y')
    parser.add_argument(dest='zsize', type=int, metavar='grid.z',
                        help='grid.z')

    parser.add_argument('--stub', default='', type=str, metavar='stub',
                        help='output file stub')

    parser.add_argument('--noise', default='uniform', type=str, metavar='str',
                        choices=['uniform', 'gaussian'], help='noise function')

    parser.add_argument('--kernel', default='gaussian', type=str,
        metavar='str', choices=['gaussian', 'random'], help='kernel function')

    parser.add_argument('--spacing', default=1.0, type=float, metavar='float',
                        help='kernel spacing')

    parser.add_argument('--sigma', default=None, metavar='float', type=float,
                        help='same as --sx # --sy # --sz #')
    parser.add_argument('--sx', default=None, metavar='float', type=float,
                        help='sigma.x for gaussian kernel')
    parser.add_argument('--sy', default=None, metavar='float', type=float,
                        help='sigma.y for gaussian kernel')
    parser.add_argument('--sz', default=None, metavar='float', type=float,
                        help='sigma.z for gaussian kernel')

    parser.add_argument('--seed', default=None, type=int, metavar='int',
                        help='random number seed')

    parser.add_argument('--lmap', action='store_true', help='apply linear map')
    parser.add_argument('--threshold', action='store_true', help='apply threshold')
    parser.add_argument('--tvalue', type=float, default=0.0, help='threshold value')

    parser.add_argument('--saven', action='store_true', help='save noise data')
    parser.add_argument('--savek', action='store_true', help='save kernel data')

    parser.add_argument('--ext', default='pkl', type=str, metavar='str',
        choices=['pkl', 'npy', 'dat', 'txt', 'csv'], help='output file type')

    return parser

def get_arguments(args=None):
    parser = create_parser()
    opts = parser.parse_args(args)

    if opts.kernel == 'gaussian':
        if opts.sx is None: opts.sx = opts.sigma
        if opts.sy is None: opts.sy = opts.sigma
        if opts.sz is None: opts.sz = opts.sigma
        if None in [opts.sx, opts.sy, opts.sz]:
            parser.print_help()
            print >> sys.stderr, ''
            print >> sys.stderr, 'must give --sx, --sy, --sz, or --sigma ' + \
                'values when using --kernel gaussian'
            sys.exit(-1)

    return opts

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    if opts.seed:
        np.random.seed(seed=opts.seed)

    grid = lm.grid.Grid(opts.xsize, opts.ysize, opts.zsize)
    print grid

    kernel = lm.surface.GaussianKernel(opts.sx, opts.sy, opts.sz,
                                       spacing=opts.spacing)
    print kernel

    if opts.noise == 'gaussian':
        rfunc = lambda size : np.random.normal(0.0, 1.0, size)
    else:
        rfunc = lambda size : np.random.random(size) - 0.5

    isotropic = lm.surface.Isotropic(grid, kernel, rfunc, verbose=True)
    print isotropic

    if opts.lmap:
        surface = lm.surface.linear_mapping(isotropic.z_image, 0, 1)
    else:
        surface = isotropic.z_image

    if opts.threshold:
        surface = lm.surface.threshold(surface, opts.tvalue)

    handle = lm.common.format_output(stub=opts.stub, name='image', ext=opts.ext)
    print 'saved: %s' % handle
    lm.surface.save(handle, surface)

    if opts.saven:
        handle = lm.common.format_output(stub=opts.stub, name='noise', ext=opts.ext)
        print 'saved: %s' % handle
        lm.surface.save(handle, isotropic.z_noise)

    if opts.savek:
        handle = lm.common.format_output(stub=opts.stub, name='kernel', ext=opts.ext)
        print 'saved: %s' % handle
        lm.surface.save(handle, isotropic.kernel.v)