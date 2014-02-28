# -*- coding: utf-8 -*-
"""
@author: adam
"""
import scipy.ndimage as ndimage
import langmuir as lm
import numpy as np
import argparse
import os

desc = """
Converts an image to trap data.
"""

def get_arguments(args=None):
    parser = argparse.ArgumentParser()
    parser.description = desc
    parser.add_argument(dest='ifile', default='image.png', type=str, nargs='?',
                        metavar='str', help='input file')
    parser.add_argument('--stub', default='sim', type=str,
                        metavar='stub', help='output file stub')
    parser.add_argument('--template', default='template.inp', type=str,
                        metavar='str', help='template input file')
    parser.add_argument('--tvalue', default=0.1, type=float, metavar='float',
                        help='trap potential')
    parser.add_argument('--rot90', default=-1, type=int, metavar='int',
                        help='rotate by n*90 degrees')
    opts = parser.parse_args(args)
    return opts

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    image = ndimage.imread(opts.ifile, flatten=True).astype(dtype=int)
    image = np.rot90(image, opts.rot90)

    image = np.expand_dims(image, 2)
    image = lm.surface.linear_mapping(image, 1, 0)
    image = lm.surface.threshold(image, v=0.5)

    xsize, ysize, zsize = image.shape
    grid = lm.grid.Grid(xsize, ysize, zsize)

    chk  = lm.checkpoint.CheckPoint.from_grid(grid)
    if os.path.exists(opts.template):
        template = lm.checkpoint.load(opts.template)
        chk.update(template)

    traps = lm.grid.IndexMapper.map_mesh(grid, image, value=1.0)
    chk.traps = traps

    chk['trap.potential'] = opts.tvalue

    print chk

    handle = lm.common.format_output(stub=opts.stub, name='', ext='inp')
    chk.save(handle)
    print 'saved: %s' % handle