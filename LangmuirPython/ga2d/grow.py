# -*- coding: utf-8 -*-
"""
grow.py
=======

.. argparse::
    :module: grow
    :func: create_parser
    :prog: grow.py

.. moduleauthor:: Geoff Hutchison <geoffh@pitt.edu>
"""

import argparse
import random
import os

from PIL import Image
import numpy as np


desc = """
Append particles to an image (adds fractal disorder to white boundaries).
"""

def create_parser():
    parser = argparse.ArgumentParser()
    parser.description = desc

    parser.add_argument(dest='ifile', type=str, metavar='input',
        help='input file')

    return parser

def get_arguments(args=None):
    parser = create_parser()
    opts = parser.parse_args(args)
    return opts

def grow_image(image, pixToAdd = -1):
    """
    .. todo:: comment function
    
    :param image: data
    :param pixToAdd: number of points to grow, -1 for automatic detection of 50:50 mix

    :type image: :py:class:`numpy.ndarray`
    :type pixToAdd: int
    """    
    # neighbor pixel directions
    nx = [-1, 1,  0, 0]
    ny = [ 0, 0, -1, 1]
    nbrs = len(nx)

    (width, height) = image.size

    if pixToAdd == -1:
        pixToAdd = width * height / 2

    # count how many are currently set
    pix = 0
    for x in range(width):
        for y in range(height):
            if image.getpixel((x, y)) > 0:
                pix += 1

    while pix < pixToAdd:
        x = random.randrange(width)
        y = random.randrange(height)
        if image.getpixel((x,y)) > 0:
            for i in range(nbrs):
                # x, y is set, so try to find an empty neighbor
                k = random.randrange(nbrs)
                xn = x + nx[k]
                yn = y + ny[k]
                if xn < 0 or xn > width - 1 or yn < 0 or yn > height - 1:
                    continue
                if image.getpixel((xn,yn)) == 0:
                    image.putpixel((xn,yn), 255)
                    pix += 1
                    break

def grow_ndimage(image, phase = 0, pixToAdd = -1):
    if phase != 0:
        inverted = (image < image.mean())
        image = inverted

    # neighbor pixel directions
    nx = [-1, 1,  0, 0]
    ny = [ 0, 0, -1, 1]
    nbrs = len(nx)

    width, height = image.shape
    if pixToAdd == -1:
        pixToAdd = width * height / 2

    # how many are currently set
    pix = np.count_nonzero(image)

    while pix < pixToAdd:
        x = random.randrange(width)
        y = random.randrange(height)
        if image[x,y] > 0:
            for i in range(nbrs):
                # x, y is set, so try to find an empty neighbor
                k = random.randrange(nbrs)
                xn = x + nx[k]
                yn = y + ny[k]
                if xn < 0 or xn > width - 1 or yn < 0 or yn > height - 1:
                    continue
                if image[xn,yn] == 0:
                    image[xn,yn] = 255
                    pix += 1
                    break

    if phase != 0:
        # re-invert to get the original phases
        inverted = (image < image.mean())
        image = inverted

    return image

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    image = Image.open(opts.ifile)
    grow_image(image)
    image.save(opts.ifile, "PNG")
