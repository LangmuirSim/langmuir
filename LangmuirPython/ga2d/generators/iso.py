# -*- coding: utf-8 -*-
"""
iso.py
======

.. argparse::
    :module: iso
    :func: create_parser
    :prog: iso.py

.. moduleauthor:: Geoff Hutchison <geoffh@pitt.edu>
"""
from scipy import misc, ndimage
import numpy as np
import argparse
import os

desc = """
gaussian noise for isotropic systems
"""

def create_parser():
    parser = argparse.ArgumentParser()
    parser.description = desc

    parser.add_argument(dest='width', default=256, type=int, metavar='dim.x',
                        nargs='?', help='dim.x')

    parser.add_argument(dest='height', default=256, type=int, metavar='dim.y',
                        nargs='?', help='dim.y')

    parser.add_argument(dest='radius', default=3, type=int, metavar='sigma',
                        nargs='?', help='radius of Gaussian blur')

    parser.add_argument('--seed', default=None, type=int, metavar='int',
                        help='random number seed')

    parser.add_argument(dest='ofile', default=None, type=str, nargs='?',
        metavar='output', help='output file')

    return parser

def get_arguments(args=None):
    parser = create_parser()
    opts = parser.parse_args(args)
    return opts

def makeIsotropic(width, height, radius = 4):
    # floating point between 0.0 - 1.0
    noise = np.random.random( (width, height) )
    scaled = ndimage.gaussian_filter(noise, sigma = radius)
    return (scaled > scaled.mean())

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    if opts.ofile is None:
        opts.ofile = "Iso-%d.png" % (opts.radius)

    if opts.seed:
        np.random.seed(seed=opts.seed)

    output = makeIsotropic(opts.width, opts.height, opts.radius)
    misc.imsave(opts.ofile, output)
