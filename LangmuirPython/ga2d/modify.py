# -*- coding: utf-8 -*-
"""
@author: Geoff Hutchison
"""
import argparse
import os
import sys
from scipy import misc, ndimage
import numpy as np
import random
import math

desc = """
mix two images and re-threshold
"""

def get_arguments(args=None):
    parser = argparse.ArgumentParser()
    parser.description = desc

    parser.add_argument(dest='ifile1', type=str, metavar='input1',
        help='input file #1')

    parser.add_argument(dest='ifile2', default="None", type=str,
                        metavar='input2', nargs='?',
                        help='input file #2')

    opts = parser.parse_args()

    if not os.path.exists(opts.ifile1):
        parser.print_help()
        print >> sys.stderr, '\nfile does not exist: %s' % opts.ifile1
        sys.exit(-1)

    return opts

def threshold(image):
    return (image > image.mean())

def blend_and_threshold(image1, image2, ratio = 0.5, radius=1):
    mixed = (ratio * image1 + (1.0 - ratio)*image2)
    image = ndimage.uniform_filter(mixed, size=radius)
    return threshold(image)

def gblur_and_threshold(image, radius=1):
    output = ndimage.gaussian_filter(image, sigma=radius)
    return threshold(output)

def ublur_and_threshold(image, radius=1):
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

def grow(image):
    width, height = image.shape
    zoom_x = random.random() + 1.0
    zoom_y = random.random() + 1.0
    child = ndimage.interpolation.zoom(image, (zoom_x, zoom_y) )
    # now take a slice of width and height out of this thing
    child_width, child_height = child.shape
    start_x = random.randrange(child_width - width)
    start_y = random.randrange(child_height - height)
    return child[ start_x:start_x+256, start_y:start_y+256 ]

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