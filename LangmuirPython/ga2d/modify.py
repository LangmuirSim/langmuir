# -*- coding: utf-8 -*-
"""
modify.py
=========

.. argparse::
    :module: modify
    :func: create_parser
    :prog: modify.py

.. moduleauthor:: Geoff Hutchison <geoffh@pitt.edu>
"""

import argparse
import os
import sys
from scipy import misc, ndimage
import numpy as np
import random
import math

desc = """
Mix two images and re-threshold or perform other mutations.
"""

def create_parser():
    parser = argparse.ArgumentParser()
    parser.description = desc

    parser.add_argument(dest='ifile1', type=str, metavar='input1',
        help='input file #1')

    parser.add_argument(dest='ifile2', default="None", type=str,
                        metavar='input2', nargs='?',
                        help='input file #2')
    return parser

def get_arguments(args=None):
    parser = create_parser()
    opts = parser.parse_args(args)

    if not os.path.exists(opts.ifile1):
        parser.print_help()
        print >> sys.stderr, '\nfile does not exist: %s' % opts.ifile1
        sys.exit(-1)

    return opts

def threshold(image):
    """
    .. todo:: comment function

    :param image: data
    :type image: :py:class:`numpy.ndarray`
    """
    return image > image.mean()

def blend_and_threshold(image1, image2, ratio = 0.5, radius=1):
    """
    .. todo:: comment function

    :param image1: data1
    :param image2: data2
    :param ratio: mixing ratio
    :param radius: size of blur kernel to use after mixing

    :type image1: :py:class:`numpy.ndarray`
    :type image2: :py:class:`numpy.ndarray`
    :type ratio: float
    :type radius: float
    """

    mixed = (ratio * image1 + (1.0 - ratio)*image2)
    image = ndimage.uniform_filter(mixed, size=radius)
    return threshold(image)

def gblur_and_threshold(image, radius=1):
    """
    .. todo:: comment function

    :param image: data
    :param radius: kernel radius

    :type image: :py:class:`numpy.ndarray`
    :type radius: float
    """
    output = ndimage.gaussian_filter(image, sigma=radius)
    return threshold(output)

def ublur_and_threshold(image, radius = 1):
    """
    .. todo:: comment function

    :param image: data
    :param radius: kernel radius

    :type image: :py:class:`numpy.ndarray`
    :type radius: float
    """
    output = ndimage.uniform_filter(image, size=radius)
    return threshold(output)

def shrink(image):
    width, height = image.shape
    shrink_x = random.uniform(0.25, 1.0)
    shrink_y = random.uniform(0.25, 1.0)
    resized = misc.imresize(image, (shrink_x * width, shrink_y * height))
    # now we need to tile this thing
    mult_x = math.ceil(1.0 / shrink_x)
    mult_y = math.ceil(1.0 / shrink_y)
    tiled = np.tile(resized, (mult_x, mult_y))
    # now take a slice of width and height out of this thing
    tiled_width, tiled_height = tiled.shape
    start_x = random.randrange(tiled_width - width)
    start_y = random.randrange(tiled_height - height)
    return tiled[ start_x:start_x+256, start_y:start_y+256 ]

def enlarge(image):
    width, height = image.shape
    zoom_x = random.random() + 1.0
    zoom_y = random.random() + 1.0
    child = ndimage.interpolation.zoom(image, (zoom_x, zoom_y) )
    # now take a slice of width and height out of this thing
    child_width, child_height = child.shape
    start_x = random.randrange(child_width - width)
    start_y = random.randrange(child_height - height)
    return child[ start_x:start_x+256, start_y:start_y+256 ]

def pepper(image, phase=0, count=0, percent=None, brush='point', overlap=False, maxTries=1e9):
    """
    Add pepper to image.

    :param image: data
    :param phase: phase ID (0, 1)
    :param count: number of pixels to change
    :param percent: percent of phase to change (overrides count!)
    :param brush: which brush to use (point, square, square3, cross)
    :param overlap: allow the brush to paint over existing pixels
    """
    # choose brush
    brushes = {
        'point': np.array([
            [0, 0]
        ]),

        'square': np.array([
            [0, 1], [1, 1],
            [0, 0], [1, 0]
        ]),

        'square3': np.array([
            [-1, 1], [0, 1], [1, 1],
            [-1, 0], [0, 0], [1, 0],
            [-1,-1], [0,-1], [1,-1]
        ]),

        'cross': np.array([
                     [0, 1],
            [-1, 0], [0, 0], [1, 0],
                     [0,-1]
        ]),
    }
    brush = brushes[brush]

    # list of unique phases
    phases = np.unique(image)
    if phases.size > 2 or phases.size == 1:
        raise ValueError, 'invalid number of phases in image: %d' % image.size

    # list of possible sites ids that can be modified
    x_i, y_i = np.where(image==phases[phase])
    phase_size = x_i.size

    # determine the number of pixels to change
    if not percent is None:
        assert 0 < percent < 1
        count = int(phase_size * percent)

    if count <= 0 or count > phase_size:
        raise ValueError, 'invalid count: %d' % count

    # add the pepper
    pixels_changed, tries = 0, 0
    while pixels_changed < count:
        # choose a random site
        i = np.random.randint(0, x_i.size - 1)

        # apply brush to site
        to_paint = brush + (x_i[i], y_i[i])
        x_j, y_j = to_paint[:,0], to_paint[:,1]

        # restrict sites to be in bounds of image
        valid = ((x_j < image.shape[0]) & (x_j >= 0) &
                 (y_j < image.shape[0]) & (y_j >= 0))
        x_j = x_j[valid]
        y_j = y_j[valid]

        # restrict sites to lie within the paiting phase
        valid = (image[x_j, y_j] == phases[phase])
        x_j = x_j[valid]
        y_j = y_j[valid]

        # allow the brush to paint over
        if overlap:
            image[x_j, y_j] = phases[phase - 1]
            pixels_changed += x_j.size

        # or restrict brush to only paint where it 'fits'
        elif x_j.size == brush.shape[0]:
            image[x_j, y_j] = phases[phase - 1]
            pixels_changed += x_j.size

        # recalcualte which sites can be painted
        x_i, y_i = np.where(image==phases[phase])
        tries += 1

        # only allow a certain number of tries
        if tries >= maxTries:
            raise RuntimeError, 'reached max tries in pepper: %d' % tries

    return image

def imshow(image):
    """
    Show image using matplotlib.
    """
    import matplotlib.pyplot as plt
    plt.autumn()
    plt.imshow(image)
    plt.tick_params(label1On=False, tick1On=False, tick2On=False)
    plt.tight_layout()
    plt.show()

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    image1 = misc.imread(opts.ifile1)
    if opts.ifile2 != "None":
        image2 = misc.imread(opts.ifile2)
        output = blend_and_threshold(image1, image2)
        misc.imsave("Blend.png", output)
    else:
        output = ublur_and_threshold(image1, 3)
        misc.imsave(opts.ifile1, output)
