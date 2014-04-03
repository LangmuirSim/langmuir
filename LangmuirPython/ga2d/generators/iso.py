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
import argparse
import os

from scipy import misc, ndimage
import numpy as np

desc = """
Generate isotropic two-phase 2D systems using Gaussian noise
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

def makeIsotropic(width, height, radius=4):
    """
    Generate a 2D isotropic two-phase system using Gaussian random noise.
    Before thresholding, the random noise is convoluted using a Gaussian blur
    kernel of sigma "radius". The resulting data will have a 50:50 mixture as
    a binary array (0, 1)

    :param width: width of the resulting data
    :param height: height of the resulting data
    :param radius: size of the Gaussian blur kernel

    :type width: int
    :type height: int
    :type radius: float

    :return: morphology data
    :rtype: :py:class:`numpy.ndarray`
    """
    # floating point between 0.0 - 1.0
    noise = np.random.random( (width, height) )
    # blur (vectorized code from ndimage)
    scaled = ndimage.gaussian_filter(noise, sigma=radius)
    # threshold, since the resulting blurred data may not have 50:50 mix
    return scaled > scaled.mean()

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    if opts.seed:
        np.random.seed(seed=opts.seed)

    if opts.ofile is None:
        opts.ofile = "iso-s%d-%d.png" % (opts.seed, opts.radius)

    output = makeIsotropic(opts.width, opts.height, opts.radius)
    misc.imsave(opts.ofile, output)
