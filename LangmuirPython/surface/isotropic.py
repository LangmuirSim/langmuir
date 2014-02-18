# -*- coding: utf-8 -*-
"""
@author: adam
"""
import langmuir as lm
import numpy as np
import argparse
import sys
import os

desc = """
Create surface using convolution.
"""

def get_arguments(args=None):
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

    if opts.noise == 'uniform':
        rfunc = lambda size : np.random.random(size) - 0.5
    elif opts.noise == 'gaussian':
        rfunc = lambda size : np.random.normal(0.0, 1.0, size)

    isotropic = lm.surface.Isotropic(grid, kernel, rfunc, verbose=True)
    print isotropic

    handle = lm.common.format_output(stub=opts.stub, name='image', ext='npy')
    print 'saved: %s' % handle
    np.save(handle, isotropic.z_image)

    handle = lm.common.format_output(stub=opts.stub, name='noise', ext='npy')
    print 'saved: %s' % handle
    np.save(handle, isotropic.z_noise)

    handle = lm.common.format_output(stub=opts.stub, name='kernel', ext='npy')
    print 'saved: %s' % handle
    np.save(handle, isotropic.kernel.v)