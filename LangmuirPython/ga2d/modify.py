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
import random
import math

from skimage import morphology
from scipy import misc, ndimage, fftpack, signal
import numpy as np
from PIL import Image

desc = """
Modify one or two images with various "mutation" options.
- Blend and re-threshold two images
- Gaussian Blur
- Uniform Blur (preserves edges)
- Shrink and tile
- Enlarge and tile
- Add pepper to a particular phase
- Roughen edges
- Grow one phase using diffusion-limited growth (fractal noise)
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

def blend_and_threshold(image1, image2, ratio=0.5, radius=2):
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

def gblur_and_threshold(image, radius=2):
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
    return output > output.mean()

def ublur_and_threshold(image, radius=2):
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
        scale_x = random.uniform(0.2, 1.0)
    if scale_y <= 0.0 or scale_y > 1.0:
        scale_y = random.uniform(0.2, 1.0)
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
        if start_x + 256 > tiled_width:
            start_x -= 1
        if start_x < 0:
            start_x = 0

    start_y = 0
    if tiled_height > height:
        start_y = random.randrange(tiled_height - height)
        if start_y + 256 > tiled_height:
            start_y -= 1
        if start_y < 0:
            start_y = 0
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
        zoom_x = 2.0*random.random() + 1.0
    if zoom_y < 1.0:
        zoom_y = 2.0*random.random() + 1.0
    child = ndimage.interpolation.zoom(image, (zoom_x, zoom_y) )

    # now take a slice of width and height out of this thing
    child_width, child_height = child.shape
    start_x = 0
    if child_width > width:
        start_x = random.randrange(child_width - width)
        if start_x < 0:
            start_x = 0

    start_y = 0
    if child_height > height:
        start_y = random.randrange(child_height - height)
        if start_y < 0:
            start_y = 0
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

def grow_ndimage(image, phase=-1, pixToAdd=-1):
    """
    Grow points into a minority phase using diffusion-limited fractal growth.
    The result will ensure 50:50 mixtures and by default will determine the
    minority phase automatically, but both could be specified.

    :param image: data
    :param phase: grey level to grow into (i.e., default 0 = add white into black regions)
    :param pixToAdd: number of points to grow, -1 for automatic detection of 50:50 mix

    :type image: :py:class:`numpy.ndarray`
    :type phase: int
    :type pixToAdd: int

    :return: modified image data
    :rtype: :py:class:`numpy.ndarray`
    """
    if phase == -1:
        nonzero = np.count_nonzero(image)
        phase = 0
        if nonzero > image.size / 2:
            phase = 255

    # switch the phases for now, we'll switch them back later
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
        # pick a spot and see if it can be added
        x = random.randrange(width)
        y = random.randrange(height)
        if image[x,y] != phase:
            for i in range(nbrs):
                # x, y is set, so try to find an empty neighbor
                k = random.randrange(nbrs)
                xn = x + nx[k]
                yn = y + ny[k]
                if xn < 0 or xn > width - 1 or yn < 0 or yn > height - 1:
                    continue
                if image[xn,yn] == phase:
                    image[xn,yn] = 255
                    pix += 1
                    break

    if phase != 0:
        # re-invert to get the original phases
        inverted = (image < image.mean())
        image = inverted

    return image

def invert(image):
    """
    Invert the phases (i.e., black -> white, white->black)

    :param image: data

    :type image: :py:class:`numpy.ndarray`

    :return: modified image data
    :rtype: :py:class:`numpy.ndarray`
    """

    return (image < image.mean())

def add_noise(image):
    """
    :param image: data
    :type image: :py:class:`numpy.ndarray`

    :return: modified image data
    :rtype: :py:class:`numpy.ndarray`
    """
    noise = np.random.random(image.shape) * 255
    return blend_and_threshold(image, noise)

def flip_random_sites(image, n=64):
    """
    What random noise should be doing?

    :param image: data
    :type image: :py:class:`numpy.ndarray`

    :param n: number of sites to flip
    :type n: int

    :return: modified image data
    :rtype: :py:class:`numpy.ndarray`
    """
    image = (image > image.mean())
    x_ids = np.random.choice(image.shape[0], size=n)
    y_ids = np.random.choice(image.shape[1], size=n)
    iflip = np.copy(image)
    iflip[x_ids, y_ids] = np.logical_not(image[x_ids, y_ids])
    return iflip > iflip.mean()

def dialate(image, struct=None):
    """
    Grow edges of white phase.

    :param image: data
    :type image: :py:class:`numpy.ndarray`

    :type struct: :py:class:`numpy.ndarray`
    :type struct: :py:class:`numpy.ndarray`

    :return: modified image data
    :rtype: :py:class:`numpy.ndarray`
    """
    if not struct:
        struct = morphology.disk(1)

    i = morphology.binary_dilation(image, struct)

    return i > i.mean()

def erode(image, struct=None):
    """
    Erode edges of white phase.

    :param image: data
    :type image: :py:class:`numpy.ndarray`

    :type struct: :py:class:`numpy.ndarray`
    :type struct: :py:class:`numpy.ndarray`

    :return: modified image data
    :rtype: :py:class:`numpy.ndarray`
    """
    if not struct:
        struct = morphology.disk(1)

    i = morphology.binary_erosion(image, struct)

    return i > i.mean()

def opening(image, struct=None):
    """
    Open up white phase so that struct fits without overlapping black phase.

    :param image: data
    :type image: :py:class:`numpy.ndarray`

    :type struct: :py:class:`numpy.ndarray`
    :type struct: :py:class:`numpy.ndarray`

    :return: modified image data
    :rtype: :py:class:`numpy.ndarray`
    """
    if not struct:
        struct = morphology.disk(1)

    i = morphology.binary_opening(image, struct)

    return i > i.mean()

def closing(image, struct=None):
    """
    Open up black phase so that struct fits without overlapping white phase.

    :param image: data
    :type image: :py:class:`numpy.ndarray`

    :type struct: :py:class:`numpy.ndarray`
    :type struct: :py:class:`numpy.ndarray`

    :return: modified image data
    :rtype: :py:class:`numpy.ndarray`
    """
    if not struct:
        struct = morphology.disk(1)

    i = morphology.binary_opening(image, struct)

    return i > i.mean()

def skeletonize_and_reconstruct(image, dfunc=None, sfunc=None):
    """
    Deconstruct image into skeleton and distance mask.  Alter the skeleton
    and/or distance mask and then reconstruct a new morphology.  With no
    altering functions passed, this function will not modify the image.

    :param image: data
    :type image: :py:class:`numpy.ndarray`

    :param dfunc: function that alters distance mask
    :type dfunc: func

    :param sfunc: function that alters skeleton
    :type sfunc: func

    :return: modified image data
    :rtype: :py:class:`numpy.ndarray`
    """
    image = (image > image.mean())
    # compute skeleton
    s0 = morphology.skeletonize(image)

    # compute mask
    d0 = ndimage.distance_transform_edt(image)

    # alter the mask
    if not dfunc is None:
        d0 = dfunc(image, s0, d0)

    # alter the skeleton
    if not sfunc is None:
        s0 = dfunc(image, s0, d0)

    # fix d-matrix
    d0[d0 <= s0] = s0[d0 <= s0]

    # recomute morphology
    r0 = morphology.reconstruction(s0, d0)

    # threshold
    return r0 > r0.mean()

def fourier_transform_and_reconstruct(image, detrend=False, window=False,
                                      ffunc=None):
    """
    Take fourier transform, alter it, and reconstruct image.  For some
    reason this is shifting the origin by 1 pixel after reconstruction, which
    should not happen.

    :param image: data
    :type image: :py:class:`numpy.ndarray`

    :param ffunc: function that alters FFT matrix
    :type ffunc: func

    :return: modified image data
    :rtype: :py:class:`numpy.ndarray`
    """
    if window:
        w = signal.hamming(image.shape)
    else:
        w = np.ones_like(image)

    if detrend:
        f = fftpack.fft(w * signal.detrend(image))
    else:
        f = fftpack.fft(w * image)

    # alter the fft
    if not ffunc is None:
        f = ffunc(f)

    result = np.fliplr(fftpack.fft(f))

    return result > result.mean()

def randf(a, b, size=None):
    """
    Random float in range.

    :param a: lower bound
    :param b: upper bound
    :type size: number of samples to generate

    :type a: float
    :type b: float
    :type size: int

    :return: values
    :rtype: float
    """
    return np.random.random(size=size) * (b - a) + a

def gauss1D(x, s=1.0, m=0.0):
    """
    1D Gaussian.

    :param x: x-value
    :param s: sigma
    :param m: mean

    :return: values
    :rtype: float
    """
    g = np.exp(-(x - m)**2 / (2.0 * s**2))
    return g

def gauss2D(x, y, sx=1.0, sy=1.0, mx=0.0, my=0.0):
    """
    2D Gaussian.

    :param x: x-value
    :param y: y-value
    :param sx: x-sigma
    :param mx: x-mean
    :param sy: y-sigma
    :param my: y-mean

    :return: values
    :rtype: float
    """
    return gauss1D(x, sx, mx) * gauss1D(y, sy, my)

def create_random_distance_transform_on_skeleton(i0, s0, d0):
    """
    Draw a bunch of Gaussians of random sigma along skeleton of image.

    :param i0: data
    :type i0: :py:class:`numpy.ndarray`

    :param s0: data
    :type s0: :py:class:`numpy.ndarray`

    :param d0: data
    :type d0: :py:class:`numpy.ndarray`
    """
    # create kernel matrix
    kx_size = 8
    ky_size = 8
    kx, ky = np.mgrid[-kx_size:1.01*kx_size:1.0,
                      -ky_size:1.01*ky_size:1.0]

    # image size
    nx, ny = i0.shape

    # skeleton x,y values
    sx, sy = np.argwhere(s0).T

    # create padded distance array
    d1 = np.zeros_like(d0)
    dp = np.pad(d1, (kx_size, ky_size), 'constant', constant_values=[0])

    # run along skeleton
    for i, (x_id, y_id) in enumerate(zip(sx, sy)):

        # jitter the skeleton
        x_id = (x_id + np.random.randint(-2, 3)) % nx
        y_id = (y_id + np.random.randint(-2, 3)) % ny

        # random sigma
        sigma_x = randf(0.1, 2.0)
        sigma_y = randf(0.1, 2.0)

        x_id_pad = x_id + kx_size
        y_id_pad = y_id + ky_size

        x0 = x_id_pad - kx_size
        x1 = x_id_pad + ky_size + 1

        y0 = y_id_pad - ky_size
        y1 = y_id_pad + ky_size + 1

        dp[x0:x1,y0:y1] += gauss2D(kx, ky, sigma_x, sigma_y)

    # remove padding
    d1 = dp[kx_size:-kx_size, ky_size:-ky_size]

    # blur edges
    d1 = ndimage.gaussian_filter(d1, sigma=1)

    # fix for reconstruction
    d1[d1 <= s0] = s0[d1 <= s0]

    return d1

def create_parser():
    parser = argparse.ArgumentParser()
    parser.description = desc

    parser.add_argument(dest='ifile1', type=str, metavar='input1',
        help='input file #1')

    parser.add_argument(dest='ifile2', default=None, type=str,
                        metavar='input2', nargs='?',
                        help='input file #2')

    parser.add_argument('--type', default=None, type=str, metavar='type',
        choices=['blend', 'gblur', 'ublur', 'shrink', 'enlarge', 'invert',
                 'pepper', 'roughen', 'fracgrow', 'noise', 'flip', 'erode',
                 'dialate', 'opening', 'closing', 'random_dist'],
        help='mutation type')

    return parser

def get_arguments(args=None):
    parser = create_parser()
    opts = parser.parse_args(args)

    if not os.path.exists(opts.ifile1):
        parser.print_help()
        print >> sys.stderr, '\nfile does not exist: %s' % opts.ifile1
        sys.exit(-1)

    if opts.ifile2 != None and not os.path.exists(opts.ifile2):
        parser.print_help()
        print >> sys.stderr, '\nfile does not exist: %s' % opts.ifile2
        sys.exit(-1)

    if opts.type == None or opts.type == "blend" and opts.ifile2 == None:
        parser.print_help()
        print >> sys.stderr, "\nYou must supply two valid filenames to blend"

    return opts

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    image = Image.open(opts.ifile1)
    image1 = misc.fromimage(image.convert("L"))

    if opts.type == 'blend':
        image = Image.open(opts.ifile2)
        image2 = misc.fromimage(image.convert("L"))
        output = blend_and_threshold(image1, image2)
        misc.imsave("Blend.png", output)
    elif opts.type == 'gblur':
        output = gblur_and_threshold(image1)
        misc.imsave(opts.ifile1, output)
    elif opts.type == 'ublur':
        output = ublur_and_threshold(image1)
        misc.imsave(opts.ifile1, output)
    elif opts.type == 'shrink':
        output = shrink(image1)
        misc.imsave(opts.ifile1, output)
    elif opts.type == 'enlarge':
        output = enlarge(image1)
        misc.imsave(opts.ifile1, output)
    elif opts.type == 'invert':
        output = invert(image1)
        misc.imsave(opts.ifile1, output)
    elif opts.type == 'pepper':
        output = pepper(image1)
        misc.imsave(opts.ifile1, output)
    elif opts.type == 'roughen':
        output = roughen(image1)
        misc.imsave(opts.ifile1, output)
    elif opts.type == 'noise':
        output = add_noise(image1)
        misc.imsave(opts.ifile1, output)
    elif opts.type == 'flip':
        output = flip_random_sites(image1)
        misc.imsave(opts.ifile1, output)
    elif opts.type == 'fracgrow':
        output = grow_ndimage(image1)
        misc.imsave(opts.ifile1, output)
    elif opts.type == 'erode':
        output = erode(image1)
        misc.imsave(opts.ifile1, output)
    elif opts.type == 'dialate':
        output = dialate(image1)
        misc.imsave(opts.ifile1, output)
    elif opts.type == 'opening':
        output = opening(image1)
        misc.imsave(opts.ifile1, output)
    elif opts.type == 'closing':
        output = closing(image1)
        misc.imsave(opts.ifile1, output)
    elif opts.type == 'random_dist':
        dfunc = create_random_distance_transform_on_skeleton
        output = skeletonize_and_reconstruct(image1, dfunc=dfunc)
        misc.imsave(opts.ifile1, output)
