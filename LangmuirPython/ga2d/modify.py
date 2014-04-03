# -*- coding: utf-8 -*-
"""
modify.py
=========

.. argparse::
    :module: modify
    :func: create_parser
    :prog: modify.py

.. todo:: add argument flags to allow command-line access to all image mutations

.. moduleauthor:: Geoff Hutchison <geoffh@pitt.edu>
"""
import argparse
import os
import sys
import random
import math

from scipy import misc, ndimage
import numpy as np
from PIL import Image

desc = """
Mix two images and re-threshold or perform other mutations.
"""

def threshold(image):
    """
    Threshold an image to return a binary :py:class:`numpy.ndarray`
    with a 50:50 mix of two phases.

    :param image: data
    :type image: :py:class:`numpy.ndarray`

    :return: modified image data
    :rtype: :py:class:`numpy.ndarray`
    """
    return image > image.mean()

def blend_and_threshold(image1, image2, ratio=0.5, radius=1):
    """
    Mix two images, blur and threshold

    :param image1: data1
    :param image2: data2
    :param ratio: mixing ratio (default is 50:50 mixture)
    :param radius: size of uniform blur kernel to use after mixing (default is no blurring)

    :type image1: :py:class:`numpy.ndarray`
    :type image2: :py:class:`numpy.ndarray`
    :type ratio: float
    :type radius: float

    :return: modified image data
    :rtype: :py:class:`numpy.ndarray`
    """
    mixed = (ratio * image1 + (1.0 - ratio)*image2)
    image = ndimage.uniform_filter(mixed, size=radius)
    return image > image.mean()

def gblur_and_threshold(image, radius=1):
    """
    Blur an image with a Gaussian kernel of supplied radius

    :param image: data
    :param radius: kernel radius (default = no blurring)

    :type image: :py:class:`numpy.ndarray`
    :type radius: float

    :return: modified image data
    :rtype: :py:class:`numpy.ndarray`
    """
    output = ndimage.gaussian_filter(image, sigma=radius)
    return image > image.mean()

def ublur_and_threshold(image, radius = 1):
    """
    Blur an image with a Uniform blur kernel of supplied radius
    (the uniform filter better preserves edges/boundaries

    :param image: data
    :param radius: kernel radius

    :type image: :py:class:`numpy.ndarray`
    :type radius: float

    :return: modified image data
    :rtype: :py:class:`numpy.ndarray`
    """
    output = ndimage.uniform_filter(image, size=radius)
    return threshold(output)

def shrink(image, scale_x=0.0, scale_y=0.0):
    """
    Randomly shrink an image using different x and y scales. Re-tile and slice
    the image to ensure the dimensions remain the same.

    :param image: data
    :param scale_x: x-dimension scale factor (or 0.0 for random choice)
    :param scale_y: y-dimension scale factor (or 0.0 for random choice)

    :type image: :py:class:`numpy.ndarray`
    :type scale_x: float
    :type scale_y: float

    :return: modified image data
    :rtype: :py:class:`numpy.ndarray`
    """
    width, height = image.shape
    if scale_x <= 0.0 or scale_x > 1.0:
        scale_x = random.uniform(0.25, 1.0)
    if scale_y <= 0.0 or scale_y > 1.0:
        scale_y = random.uniform(0.25, 1.0)
    resized = misc.imresize(image, (int(scale_x * width), int(scale_y * height)) )
    # now we need to tile this thing, so work out how many repeats are needed
    mult_x = math.ceil(1.0 / scale_x)
    mult_y = math.ceil(1.0 / scale_y)
    tiled = np.tile(resized, (mult_x, mult_y))

    # now take a slice of width and height out of the re-tiled version
    tiled_width, tiled_height = tiled.shape
    start_x = 0
    if tiled_width > width:
        start_x = random.randrange(tiled_width - width)
    start_y = 0
    if tiled_height > height:
        start_y = random.randrange(tiled_height - height)
    return tiled[ start_x:start_x+256, start_y:start_y+256 ]

def enlarge(image, zoom_x=0.0, zoom_y=0.0):
    """
    Randomly enlarge an image using different x and y scales. Slice the image to
    ensure the dimensions remain the same.

    :param image: data
    :param zoom_x: x-dimension scale factor (or 0.0 for random choice)
    :param zoom_y: y-dimension scale factor (or 0.0 for random choice)

    :type image: :py:class:`numpy.ndarray`
    :type zoom_x: float
    :type zoom_y: float

    :return: modified image data
    :rtype: :py:class:`numpy.ndarray`
    """
    width, height = image.shape
    if zoom_x < 1.0:
        zoom_x = random.random() + 1.0
    if zoom_y < 1.0:
        zoom_y = random.random() + 1.0
    child = ndimage.interpolation.zoom(image, (zoom_x, zoom_y) )

    # now take a slice of width and height out of this thing
    child_width, child_height = child.shape
    start_x = 0
    if child_width > width:
        start_x = random.randrange(child_width - width)
    start_y = 0
    if child_height > height:
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

    :return: modified image data
    :rtype: :py:class:`numpy.ndarray`
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
    if brush in brushes:
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

    :param image: data

    :type image: :py:class:`numpy.ndarray`
    """
    import matplotlib.pyplot as plt
    plt.autumn()
    plt.imshow(image)
    plt.tick_params(label1On=False, tick1On=False, tick2On=False)
    plt.tight_layout()
    plt.show()

def roughen(image, fraction=0.5, dilation=0, struct=None):
    """
    Roughen the edges of the two phases. Use a binary-closing and dilation
    to find the edge between phases (ignoring pepper defects) and then flip
    the phase of a fraction of the resulting edge sites.

    :param image: data
    :param fraction: percent of sites to flip
    :param dilation: number of times to dilate (expand) the border
    :param struct: structure used for the mathematical morphology operations

    :type image: :py:class:`numpy.ndarray`
    :type fraction: float
    :type dilation: int
    :type struct: :py:class:`numpy.ndarray`

    :return: modified image data
    :rtype: :py:class:`numpy.ndarray`
    """
    if struct==None:
        struct = ndimage.morphology.generate_binary_structure(2, 1)

    # remove small noise with a binary closing
    cleaned = ndimage.binary_closing(image, structure=struct)
    # find the edge with a dilation
    edge = np.logical_xor( cleaned, ndimage.binary_dilation(cleaned, structure=struct) )
    # optionally dilate the edge (extending into both phases)
    for i in range(dilation):
        edge = ndimage.binary_dilation(edge, structure=struct)
    # add noise to the edge sites
    # this is probably inefficient, but it works
    sites = int(np.count_nonzero(edge) * fraction)
    x_nz, y_nz = edge.nonzero() # get all the nonzero indices
    count  = 0
    while count < sites:
        x = random.choice(x_nz)
        y = random.choice(y_nz)
        if (edge[x,y] != 0):
            edge[x, y] = 0 # invert one of these edge sites
            count += 1
    return np.logical_xor( image, edge )

def create_parser():
    parser = argparse.ArgumentParser()
    parser.description = desc

    parser.add_argument(dest='ifile1', type=str, metavar='input1',
        help='input file #1')

    parser.add_argument(dest='ifile2', default=None, type=str,
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

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    image = Image.open(opts.ifile1)
    image1 = misc.fromimage(image.convert("L"))

    if not opts.ifile2 is None:
        image2 = misc.imread(opts.ifile2)
        output = blend_and_threshold(image1, image2)
        misc.imsave("Blend.png", output)
    else:
#        output = ublur_and_threshold(image1, 3)
#        output = ndimage.binary_closing(image1.astype(np.uint8), structure=ndimage.morphology.generate_binary_structure(2, 1) )
        output = morphology.skeletonize( (image1 > image1.mean()) )
        misc.imsave("skel.png", output)
        # get the distances
        dists = ndimage.distance_transform_edt(image1)
        # ok for all the nonzero in the skeleton, we get the distances
        x_nz, y_nz = output.nonzero() # get all the nonzero indices
        channels = []
        for i in range(len(x_nz)):
            x = x_nz[i]
            y = y_nz[i]
            channels.append(dists[x,y])

        width4 = 0
        width2 = 0
        for i in channels:
            if i <= 4:
                width4 += 1
            if i <= 2:
                width2 += 1

        print width4, width2

#        x = np.logical_xor(image1, output)
#        misc.imsave("xor.png", x)
