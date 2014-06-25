# -*- coding: utf-8 -*-
"""
ga_analyze.py
=============

.. moduleauthor:: Geoff Hutchison <geoffh@pitt.edu>
"""
import sys
import argparse
from collections import deque
from multiprocessing import Pool

from scipy import ndimage, misc
from skimage import morphology
import numpy as np
from PIL import Image
import itertools
import time
import os

# our code
import modify

desc = """
Image analysis for various "descriptors" of morphology performance. Allows rapid GA evolution of morphology images.
"""

def as_ndarray_3D(obj):
    """
    Makes sure object is an ndarray with len(shape) = 3

    :param obj: arbitrary python object
    :return: :py:class:`numpy.ndarray`
    """
    array = np.asanyarray(obj, dtype=np.float64)

    ndims = len(array.shape)
    if ndims >= 3:
        return array

    if ndims == 2:
        return np.expand_dims(array, 2)

    if ndims == 1:
        return np.expand_dims(np.expand_dims(array, 1), 1)

    return array

def as_ndarray_2D(obj):
    """
    Makes sure object is an ndarray with len(shape) = 2

    :param obj: arbitrary python object
    :return: :py:class:`numpy.ndarray`
    """
    array = np.asanyarray(obj, dtype=np.float64)

    ndims = len(array.shape)

    if ndims > 3:
        raise RuntimeError('object has too many dimensions (%d)' % ndims)

    if ndims == 3:
        if 1 in array.shape:
            index = array.shape.index(1)
            array = np.rollaxis(array, index, 0)
            return array[0,:,:]

        raise RuntimeError('can not collapse dimensions, shape=(%d, %d, %d)' % array.shape)

    if ndims == 2:
        return array

    return np.expand_dims(array, 1)

def load_image(filename, make3D=False):
    """
    Loads data from image file (png, jpg, etc).

    :param filename: name of file
    :param make3D: forces object to be ndarray with len(shape)=3

    :type filename: str
    :type make3D: bool

    :return: :py:class:`numpy.ndarray`
    """
    pil_img = Image.open(filename)
    image = misc.fromimage(pil_img.convert("L"))
    return image

def load_npy(filename, make3D=False):
    """
    Loads data from numpy file (npy).

    :param filename: name of file
    :param make3D: forces object to be ndarray with len(shape)=3

    :type filename: str
    :type make3D: bool

    :return: :py:class:`numpy.ndarray`
    """
    image = np.load(filename)
    return image

# from http://scipy-lectures.github.io/advanced/image_processing/
def disk_structure(n):
    """
    Generate a mathematical morphology structure kernel (i.e., a circle/disk)
    of radius n

    :param n: radius of disk
    :type n: int

    :return: structuring kernel
    :rtype: :py:class:`numpy.ndarray`

    .. seealso:: :func:`square_structure()`
    """
    struct = np.zeros((2 * n + 1, 2 * n + 1))
    x, y = np.indices((2 * n + 1, 2 * n + 1))
    mask = (x - n)**2 + (y - n)**2 <= n**2
    struct[mask] = 1
    return struct.astype(np.bool)

def square_structure(n):
    """
    Generate a mathematical morphology structure kernel (i.e., a square)
    of size n

    :param n: radius of disk
    :type n: int

    :return: structuring kernel
    :rtype: :py:class:`numpy.ndarray`

    .. seealso:: :func:`disk_structure()`
    """
    struct = np.ones((n,n), dtype=np.bool)
    return struct

# from http://scipy-lectures.github.io/advanced/image_processing/
def granulometry(image, sizes=None, structure=disk_structure):
    """
    .. todo:: comment function

    :param image: data
    :param sizes: list of particle sizes to test

    :type data: :py:class:`numpy.ndarray`
    :type sizes: list[int]

    :return: list of particle counts at each size
    :rtype: list[int]
    """
    s = max(image.shape)
    if sizes == None:
        sizes = range(1, s/2, 2)
    granulo = [ndimage.binary_opening(image,
               structure=structure(n)).sum() for n in sizes]
    return granulo

def average_domain_size(image):
    """
    Calculate the average domain size for the image (for the "true" phase)

    :param image: data
    :type image: :py:class:`numpy.ndarray`

    :return: mean and standard deviation of domain sizes
    :rtype: tuple(int)
    """
    # get the average size of domains (i.e., distance from center of domain to other phase)
    # first use ndimage to label each domain
    labels, nb_labels = ndimage.label(image)

    # print "Number of domains: ", nb_labels

    # get the Euclidian distance to the other phase
    dists = ndimage.distance_transform_edt(image)
    # for each domain, get the maximum distance (i.e., the size of that domain)
    imax = ndimage.measurements.maximum(dists, labels,
        index=np.arange(1, nb_labels + 1))
    # return the mean of the max array
    return np.mean(imax), np.std(imax)

def box_counting_dimension(image):
    """
    Calculate the fractal dimension of the edges of the supplied image.

    :param image: data
    :type image: :py:class:`numpy.ndarray`

    :return: slope of best-fit line of the log-log plot
    :rtype: int

    .. seealso:: `Wikipedia <http://en.wikipedia.org/wiki/Minkowski–Bouligand_dimension>`_
    """
    width, height = image.shape
    sizes = [ 2, 4, 8, 16, 32, 64 ]
    grains = []
    for size in sizes:
        count = 0
        for x in range(0, width, size):
            for y in range(0, height, size):
                nz = np.count_nonzero(image[x:x+size, y:y+size])
                if nz != 0 and nz != size*size:
                    count += 1
        grains.append(count)

    return np.polyfit(np.log(sizes), np.log(grains), 1)

def interface_size(image, periodic=False):
    """
    Calculate the interfacial area (length for 2D images) between the phases.
    Faster version of :py:func:`interface_size_old` that works on any dimension.  Tested for 1D, 2D, and 3D.

    :param image: data
    :param periodic: use periodic boundary conditions

    :type image: :py:class:`numpy.ndarray`
    :type periodic: bool

    :return: total length of interface (in pixels)
    :rtype: int

    ..seealso: :py:func:`test_interface_size`, :py:func:`interface_size_old`
    """
    interface = 0

    # works by shifting all elements in the array by +1 and -1 (for each dimension)
    # the shifted array is compared to the original to determine if and interface exists
    # removing the slicing means periodic boundary conditions are used

    if periodic:
        for dim in range(image.ndim):
            interface += np.sum(np.where((np.roll(image, shift=+1, axis=dim) - image) > 0, 1, 0))
            interface += np.sum(np.where((np.roll(image, shift=-1, axis=dim) - image) > 0, 1, 0))
    else:
        for dim in range(image.ndim):
            s = [slice(+1, None) if i == dim else Ellipsis for i in range(image.ndim)]
            interface += np.sum(np.where((np.roll(image, shift=+1, axis=dim) - image) > 0, 1, 0)[s])

            s = [slice(None, -1) if i == dim else Ellipsis for i in range(image.ndim)]
            interface += np.sum(np.where((np.roll(image, shift=-1, axis=dim) - image) > 0, 1, 0)[s])

    return interface

def transfer_distance(original, axis=0, rot90=1):
    """
    Calculate the connectivity of the two phases to the side electrodes and
    the average "transfer distance" (i.e., the shortest distance a charge carrier
    must travel to be collected at an electrode).

    The transfer distances and connectivity fractions are calculated using a breadth-first
    search from the electrodes left = white phase, right = black phase.

    :param original: data
    :type original: :py:class:`numpy.ndarray`

    :param axis: transfer axis (x=0, y=1, z=2)
    :type axis: int

    :param rot90: rotate image
    :type rot90: int

    :return: average transfer distance and connectivity fraction (phase 1 and then phase 2)
    :rtype: tuple(float)
    """
    # the image will come in with row-major order (i.e., numpy)
    # but we're thinking of this as a graphic, with column, row
    if rot90 != 0:
        image = np.rot90(original, rot90)
    else:
        image = original

    # initialize the distance matrix
    # In NumPy 1.8 (not used)   distances.full( (width, height), -1.0)
    distances = np.empty_like(image, dtype=np.float64)
    distances.fill(-1.0)

    # ok, so it's a deque not a queue..
    # .. it still works for our purposes and Python doesn't have a queue
    work = deque()

    # begin index magic
    L_index = [+0 if i == axis else slice(None) for i in range(image.ndim)] # slice along "left" edge of image
    R_index = [-1 if i == axis else slice(None) for i in range(image.ndim)] # slice along "right" edge of image

    L_white_index = np.where(image[L_index] != 0) # sites that are white along "left" edge
    R_black_index = np.where(image[R_index] == 0) # sites that are black along "right" edge

    # alter slicing index so we can set the correct distances to zero
    j = 0
    for i in range(image.ndim):
        if i == axis:
            pass
        else:
            L_index[i] = L_white_index[j]
            R_index[i] = R_black_index[j]
            j += 1

    distances[L_index] = 0
    distances[R_index] = 0

    # alter slicing index so we can update the work queue
    for i in range(image.ndim):
        if i == axis:
            L_index[i] = itertools.repeat(0)
            R_index[i] = itertools.repeat(image.shape[i] - 1)
        else:
            pass
    L_index.append(itertools.repeat(255))
    R_index.append(itertools.repeat(0))

    # update work queue
    work.extend(zip(*L_index))
    work.extend(zip(*R_index))

    # neighbor list
    neighbors = np.zeros((2 * image.ndim, image.ndim), dtype=np.int32)
    for i in range(image.ndim):
        neighbors[2 * i + 0, i] = +1
        neighbors[2 * i + 1, i] = -1

    # compute distance matrix
    while work:
        work_item = work.popleft()
        pixel = work_item[:-1]
        phase = work_item[-1]
        currentDist = distances[pixel]

        shift = pixel + neighbors
        for i in range(image.ndim):
            shift = shift[(shift[:,i] >= 0) & (shift[:,i] < image.shape[i])]

        for indx in shift:
            indx = tuple(indx)

            if image[indx] == phase:
                if distances[indx] == -1:
                    # first visit
                    distances[indx] = currentDist + 1
                    work.append(indx + (phase,))

                elif distances[indx] > (currentDist + 1):
                    # somehow found a shorter path
                    distances[indx] = currentDist + 1

    avg1 = np.average(distances[(image != 0) & (distances >= 0)])
    avg2 = np.average(distances[(image == 0) & (distances >= 0)])

    #std1 = np.std(distances[(image != 0) & (distances >= 0)])
    #std2 = np.std(distances[(image == 0) & (distances >= 0)])

    con1 = np.average(distances[image != 0] >= 0)
    con2 = np.average(distances[image == 0] >= 0)

    return avg1, con1, avg2, con2

def bottleneck_distribution(image):
    """
    Count the distribution of bottlenecks

    :param image: data (binary)
    :type image: :py:class:`numpy.ndarray`

    :return: count of bottlenecks of size 4 and size 2
    :rtype: tuple(int)
    """
    skel = morphology.skeletonize(image)
    # get the distances
    dists = ndimage.distance_transform_edt(image)
    # ok for all the nonzero in the skeleton, we get the distances
    x_nz, y_nz = skel.nonzero() # get all the nonzero indices
    width4 = 0
    width2 = 0
    for i in range(len(x_nz)):
        x = x_nz[i]
        y = y_nz[i]

        dist = dists[x,y]
        if dist <= 4:
            width4 += 1
        if dist <= 2:
            width2 += 1

    return width4, width2

def analyze(filename):
    pil_img = Image.open(filename)
    image = misc.fromimage(pil_img.convert("L"))

    isize = interface_size(image)

    # transfer distances
    # connectivity
    td1, connect1, td2, connect2 = transfer_distance(image)

    # get the domain size of the white phase
    # then invert the image to get the black domain size
    image = (image > image.mean())
    ads1, std1 = average_domain_size(image)
    inverted = (image < image.mean())
    ads2, std2 = average_domain_size(inverted)

    # bottlenecks of phase1, phase2
    b41, b21 = bottleneck_distribution(image)
    b42, b22 = bottleneck_distribution(inverted)

    # fraction of phase one
    phase1 = np.count_nonzero(image)
    fraction = float(phase1) / float(image.size)

    # Fractal dimension by box counting -- get the slope of the log/log fit
    bcd = -1.0* box_counting_dimension(image)[0]

    # count the number of pepper defects
    spots = np.logical_xor( image, ndimage.binary_erosion(image, structure=np.ones((2,2))) )
    erosion_spots = np.count_nonzero(spots)
    spots = np.logical_xor( image, ndimage.binary_dilation(image, structure=np.ones((2,2))) )
    dilation_spots = np.count_nonzero(spots)
    spots = np.logical_xor( image, ndimage.binary_closing(image, structure=ndimage.morphology.generate_binary_structure(2, 1)) )
    closure_spots = np.count_nonzero(spots)

    blur = modify.ublur_and_threshold(image, 3)
    x = np.logical_xor(image, blur)
    nzBlur = np.count_nonzero(x)

    results = {k : v for k, v in locals().iteritems() if k in keys}
    return ' '.join(['{%s:%s}' % (k, f) for k, f in zip(keys, fmts)]).format(**results)

columns = [
    ('Filename', 'filename', '>8'),
    ('AvgDomainSize1', 'ads1', '>8.4f'),
    ('StdDevDom1', 'std1', '>8.4f'),
    ('AvgDomainSize2', 'ads2', '>8.4f'),
    ('StdDevDom2', 'std1', '>8.4f'),
    ('InterfaceSize', 'isize', '>8d'),
    ('AvgTransDist1', 'td1', '>8.4f'),
    ('Connect1', 'connect1', '>8.4f'),
    ('AvgTransDist2', 'td2', '>8.4f'),
    ('Connect2', 'connect2', '>8.4f'),
    ('PhaseRatio', 'fraction', '>8.4f'),
    ('BoxCDim', 'bcd', '>8.4f'),
    ('Bottle4-1', 'b41', '>8d'),
    ('Bottle2-1', 'b21', '>8d'),
    ('Bottle4-2', 'b42', '>8d'),
    ('Bottle2-2', 'b22', '>8d'),
    ('Erosion', 'erosion_spots', '>8d'),
    ('Dilation', 'dilation_spots', '>8d'),
    ('Closure', 'closure_spots', '>8d'),
    ('Blur3', 'nzBlur', '>8d'),
]
titles, keys, fmts = zip(*columns)

def create_parser():
    parser = argparse.ArgumentParser()
    parser.description = desc

    parser.add_argument(dest='images', nargs='*', metavar='image.png',
        help='png files to analyze')

    parser.add_argument('--output', default=None, help='output file name')

    parser.add_argument('-r', action='store_true',
        help='search for images recursivly')

    return parser

def get_arguments(args=None):
    parser = create_parser()
    opts = parser.parse_args(args)

    if not opts.images:
        possible_ext = ['.png', '.PNG']

        work = os.getcwd()
        if opts.r:
            for root, dirs, files in os.walk(work):
                for f in files:
                    stub, ext = os.path.splitext(f)
                    if ext in possible_ext:
                        opts.images.append(os.path.join(root, f))
        else:
            root, dirs, files = os.walk(work).next()
            for f in files:
                stub, ext = os.path.splitext(f)
                if ext in possible_ext:
                    opts.images.append(os.path.join(root, f))

    if not opts.images:
        print >> sys.stderr, 'could not find any png files! (use -r ???)'
        sys.exit(-1)
    else:
        for img in opts.images:
            if not os.path.exists(img):
                print >> sys.stderr, 'file does not exist:\n\t%s' % img
                sys.exit(-1)
        opts.images = [os.path.relpath(i) for i in opts.images]

    return opts

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    titles = ' '.join(['%8s' % t for t in titles])

    # create a thread pool
    pool = Pool()

    if pool is None:
        output = map(analyze, opts.images)
    else:
        output = pool.map(analyze, opts.images)

    if opts.output is None:
        print titles
        for line in output:
            print line
    else:
        with open(opts.output, 'w') as handle:
            print >> handle, titles
            for line in output:
                print >> handle, line