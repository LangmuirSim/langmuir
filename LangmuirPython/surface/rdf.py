# -*- coding: utf-8 -*-
"""
rdf.py
======

.. argparse::
    :module: rdf
    :func: create_parser
    :prog: rdf.py

.. moduleauthor:: Adam Gagorik <adam.gagorik@gmail.com>
"""
import collections
import argparse
import os

import numpy as np

import langmuir as lm


desc = """
Perform RDF on surface or KPFM image.  This script takes some time.
"""

def create_parser():
    parser = argparse.ArgumentParser()
    parser.description = desc

    parser.add_argument(dest='ifile', type=str, metavar='input',
        help='input file')

    parser.add_argument('--stub', default='', type=str, metavar='stub',
        help='output file stub')

    return parser

def get_arguments(args=None):
    parser = create_parser()
    opts = parser.parse_args(args)
    return opts

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    image = lm.surface.load(opts.ifile)
    image = image / np.sum(image)

    xsize = image.shape[0]
    ysize = image.shape[1]
    zsize = image.shape[2]
    grid  = lm.grid.Grid(xsize, ysize, zsize)
    mesh  = lm.grid.PrecalculatedMesh(grid)

    w1 = np.zeros(grid.shape)
    w2 = np.zeros(grid.shape)
    xi, yi, zi = mesh.mxi, mesh.myi, mesh.mzi
    for i, ((xj, yj, zj), vi) in enumerate(np.ndenumerate(image)):
        dx = abs(xj - xi)
        dy = abs(yj - yi)
        dz = abs(zj - zi)
        w1[dx,dy,dz] += image[xi,yi,zi]
        w2[dx,dy,dz] += image[xj,yj,zj]
        if i % 100 == 0:
            print '%.5f %%' % ((i + 1)/float(grid.size))
    print '%.5f %%' % 1

    results = collections.OrderedDict()
    results['image'] = image
    results['r1'] = mesh.r1
    results['w1'] = w1
    results['w2'] = w2

    handle = lm.common.format_output(stub=opts.stub, name='rdf', ext='pkl')
    lm.common.save_pkl(results, handle)
    print 'saved: %s' % handle