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


# our code
import modify

desc = """
Image analysis for various "descriptors" of morphology performance. Allows rapid GA evolution of morphology images.
"""

def create_parser():
    parser = argparse.ArgumentParser()
    parser.description = desc

    return parser

def get_arguments(args=None):
    parser = create_parser()
    opts = parser.parse_args(args)
    return opts

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

    if make3D:
        return as_ndarray_3D(image)
    return as_ndarray_2D(image)

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

    if make3D:
        return as_ndarray_3D(image)
    return as_ndarray_2D(image)

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

    :return mean and standard deviation of domain sizes
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

    :return slope of best-fit line of the log-log plot
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

def interface_size(image):
    """
    Calculate the interfacial area (length for 2D images) between the phases.

    :param image: data
    :type image: :py:class:`numpy.ndarray`

    :return total length of interface (in pixels)
    :rtype int
    """
    ndims = len(image.shape)

    if ndims == 2:
        # loop through the image to count the number of interfaces
        interface = 0

        # we only need to count sites to the left or above the current site
        nx = [-1,  0]
        ny = [ 0, -1]
        nbrs = len(nx)

        width, height = image.shape
        for x in range(width):
            for y in range(height):
                if image[x,y] == 0:
                    for nbr in range(nbrs):
                        xn = x + nx[nbr]
                        yn = y + ny[nbr]
                        if xn < 0 or yn < 0:
                            continue
                        if image[xn,yn] > 0:
                            interface += 1
                else:
                    for nbr in range(nbrs):
                        xn = x + nx[nbr]
                        yn = y + ny[nbr]
                        if xn < 0 or yn < 0:
                            continue
                        if image[xn,yn] == 0:
                            interface += 1

        return interface

    elif ndims == 3:
        interface = 0

        nx = [-1, 0, 0]
        ny = [ 0,-1, 0]
        nz = [ 0, 0,-1]
        nbrs = len(nx)

        width, height, depth = image.shape
        for x in range(width):
            for y in range(height):
                for z in range(depth):
                    if image[x,y,z] == 0:
                        for nbr in range(nbrs):
                            xn = x + nx[nbr]
                            yn = y + ny[nbr]
                            zn = z + nz[nbr]
                            if xn < 0 or yn < 0 or zn < 0:
                                continue
                            if image[xn,yn,zn] > 0:
                                interface += 1
                    else:
                        for nbr in range(nbrs):
                            xn = x + nx[nbr]
                            yn = y + ny[nbr]
                            zn = z + nz[nbr]
                            if xn < 0 or yn < 0 or zn < 0:
                                continue
                            if image[xn,yn,zn] == 0:
                                interface += 1
        return interface

    raise RuntimeError('interface_size: invalid dimension!')

def transfer_distance(original):
    """
    Calculate the connectivity of the two phases to the side electrodes and
    the average "transfer distance" (i.e., the shortest distance a charge carrier
    must travel to be collected at an electrode).

    The transfer distances and connectivity fractions are calculated using a breadth-first
    search from the electrodes left = white phase, right = black phase.

    :param original: data
    :type original: :py:class:`numpy.ndarray`

    :return average transfer distance and connectivity fraction (phase 1 and then phase 2)
    :rtype tuple(float)
    """

    # the image will come in with row-major order (i.e., numpy)
    # but we're thinking of this as a graphic, with column, row
    image = np.rot90(original)
    width, height = image.shape

    # initialize the distance matrix
    # In NumPy 1.8 (not used)   distances.full( (width, height), -1.0)
    distances = np.empty(width*height)
    distances.fill(-1.0)
    distances.shape = (width, height)

    # ok, so it's a deque not a queue..
    # .. it still works for our purposes and Python doesn't have a queue
    work = deque()
    nx = [-1, 1,  0, 0]
    ny = [ 0, 0, -1, 1]
    nbrs = len(nx)

    # we make two passes.
    # first, from the top left for the white (>0) phase
    for y in range(height):
        if image[0,y] > 0:
            # set the distance and add to the work queue
            distances[0,y] = 0
            work.append( (0, y, 255) )

    # now from the bottom right for the black phase
    for y in range(height - 1, 0, -1):
        if image[width - 1, y] == 0:
            # set the distance and add to the work queue
            distances[width - 1, y] = 0
            # push x,y, phase onto the queue
            work.append( (width - 1, y, 0) )

    # go through the work queue
    while work:
        x, y, phase = work.popleft()
        currentDist = distances[x, y]
        # check neighbors
        for nbr in range(nbrs):
            xn = x + nx[nbr]
            yn = y + ny[nbr]
            if xn < 0 or xn > width -1 or yn < 0 or yn > height - 1:
                continue
            # check the phase of this neighbor
            if image[xn, yn] == phase:
                if distances[xn,yn] == - 1:
                    # first visit
                    distances[xn,yn] = currentDist + 1
                    work.append( (xn, yn, phase) )
                elif distances[xn,yn] > (currentDist + 1):
                    # somehow found a shorter path
                    distances[xn,yn] = currentDist + 1

    # compute the average distances for each phase
    avgDistance1 = 0.0
    avgDistance2 = 0.0
    count1 = 0
    count2 = 0
    unconnectedCount1 = 0
    unconnectedCount2 = 0
    for x in range(width):
        for y in range(height):
            if image[x,y] != 0:
                # white phase
                if distances[x,y] >= 0.0:
                    avgDistance1 += distances[x,y]
                    count1 += 1
                else:
                    unconnectedCount1 += 1
            else:
                if distances[x,y] >= 0.0:
                    avgDistance2 += distances[x,y]
                    count2 += 1
                else:
                    unconnectedCount2 += 1

    if count1 != 0:
        avgDistance1 = avgDistance1 / float(count1)
    else:
        avgDistance1 = 0
    phase1 = np.count_nonzero(image)
    connectivity1 = 1.0 - float(unconnectedCount1) / float(phase1)

    if count2 != 0:
        avgDistance2 = avgDistance2 / float(count2)
    else:
        avgDistance2 = 0
    connectivity2 = 1.0 - float(unconnectedCount2) / float(width * height - phase1)

    return avgDistance1, connectivity1, avgDistance2, connectivity2

def bottleneck_distribution(image):
    """
    Count the distribution of bottlenecks

    :param image: data (binary)
    :type image: :py:class:`numpy.ndarray`

    :return count of bottlenecks of size 4 and size 2
    :rtype tuple(int)
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

if __name__ == '__main__':
    # create a thread pool
    pool = Pool()

    print ' '.join(['%8s' % t for t in titles])

    if pool is None:
        output = map(analyze, sys.argv[1:])
    else:
        output = pool.map(analyze, sys.argv[1:])

    for line in output:
        print line
