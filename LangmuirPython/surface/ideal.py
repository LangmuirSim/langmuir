# -*- coding: utf-8 -*-
"""
ideal.py
========

.. argparse::
    :module: ideal
    :func: create_parser
    :prog: ideal

.. moduleauthor:: Adam Gagorik <adam.gagorik@gmail.com>
"""
import langmuir as lm
import collections
import argparse
import sys
import os

surfaces = collections.OrderedDict()
surfaces['gyroid' ] = lm.surface.gyroid
surfaces['scherk' ] = lm.surface.scherk_first_surface
surfaces['p_surf' ] = lm.surface.schwarz_p_surface
surfaces['d_surf' ] = lm.surface.schwarz_d_surface
surfaces['band_xy'] = lm.surface.bandXY
surfaces['band_yz'] = lm.surface.bandYZ
surfaces['band_xz'] = lm.surface.bandXZ
surfaces['sin_x'  ] = lm.surface.sin_x
surfaces['sin_y'  ] = lm.surface.sin_y
surfaces['sin_z'  ] = lm.surface.sin_z
surfaces['cos_x'  ] = lm.surface.cos_x
surfaces['cos_y'  ] = lm.surface.cos_y
surfaces['cos_z'  ] = lm.surface.cos_z

desc = """
Create surface using ideal function.
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

    parser.add_argument(dest='xwidth', type=int, metavar='int',
                        help='size of bands, x-direction: lambda.x')
    parser.add_argument(dest='ywidth', type=int, metavar='int',
                        help='size of bands, y-direction: lambda.y')
    parser.add_argument(dest='zwidth', type=int, metavar='int',
                        help='size of bands, z-direction: lambda.z')

    parser.add_argument(dest='surface', type=str, metavar='str',
                        choices=surfaces.keys(), help='surface type')

    parser.add_argument('--stub', default='', type=str, metavar='stub',
                        help='output file stub')

    parser.add_argument('--spacing', default=1.0, type=float, metavar='float',
                        help='grid spacing')

    parser.add_argument('--lmap', action='store_true', help='apply linear map')
    parser.add_argument('--threshold', action='store_true', help='apply threshold')
    parser.add_argument('--tvalue', type=float, default=0.0, help='threshold value')

    parser.add_argument('--ext', default='pkl', type=str, metavar='str',
        choices=['chk', 'pkl', 'npy', 'dat', 'txt', 'csv'], help='output file type')

    return parser

def get_arguments(args=None):
    parser = create_parser()
    opts = parser.parse_args(args)

    if any([not opts.xsize % opts.xwidth == 0,
            not opts.ysize % opts.ywidth == 0,
            not opts.zsize % opts.zwidth == 0]):
        parser.print_help()
        print >> sys.stderr, ''
        print >> sys.stderr, 'lambda must be a multiple of grid size'
        sys.exit(-1)
    
    if opts.ext in ['chk', 'inp']:
        if not opts.threshold:
            opts.threshold = True
            message = '\n%s: using %s with %s = %s\n'
            print >> sys.stderr, message % (lm.common.ccodes.red('warning'),
                lm.common.ccodes.yellow('--threshold'),
                lm.common.ccodes.yellow('--tvalue'),
                lm.common.ccodes.cyan('%.1f' % opts.tvalue))
            lm.common.ccodes.blue

    return opts

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    grid = lm.grid.Grid(opts.xsize, opts.ysize, opts.zsize)
    grid.refine(1.0/opts.spacing)
    print grid

    wx = lm.surface.WaveDimensions(grid.lx, grid.lx/ opts.xwidth)
    wy = lm.surface.WaveDimensions(grid.ly, grid.ly/ opts.ywidth)
    wz = lm.surface.WaveDimensions(grid.lz, grid.lz/ opts.zwidth)

    print wx
    print wy
    print wz

    surface = surfaces[opts.surface](grid.mx, grid.my, grid.mz, wx, wy, wz)

    if opts.lmap:
        surface = lm.surface.linear_mapping(surface, 0, 1)

    if opts.threshold:
        surface = lm.surface.threshold(surface, opts.tvalue)

    handle = lm.common.format_output(stub=opts.stub, name='image', ext=opts.ext)
    print 'saved: %s' % handle
    lm.surface.save(handle, surface)
