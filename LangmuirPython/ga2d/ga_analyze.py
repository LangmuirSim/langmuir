# -*- coding: utf-8 -*-
"""
ga_analyze.py
=============

.. moduleauthor:: Geoff Hutchison <geoffh@pitt.edu>
"""
import sys
import argparse
from collections import deque

from scipy import ndimage, misc
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

    return interface

def transfer_distance(original):
    """
    Calculate the connectivity of the two phases to the side electrodes and
    the average "transfer distance" (i.e., the shortest distance a charge carrier
    must travel to be collected at an electrode).

    The transfer distances and connectivity fractions are calculated using a breadth-first
    search from the electrodes left = white phase, right = black phase.

    :param original: data
    :type original: :py:class:`numpy.ndarray`

    :return average transfer distance and connectivity fraction
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

    # compute the average distances
    avgDistance = 0.0
    count = 0
    unconnectedCount = 0
    for x in range(width):
        for y in range(height):
            if distances[x,y] >= 0.0:
                avgDistance += distances[x,y]
                count += 1
            else:
                unconnectedCount += 1

    avgDistance = avgDistance / float(count)
    connectivityFraction = 1.0 - float(unconnectedCount) / float(width * height)

    return avgDistance, connectivityFraction

if __name__ == '__main__':

    print "Filename, AvgDomainSize1, StdDevDom1, AvgDomainSize2, StdDevDom2, InterfaceSize, AvgTransferDist, "\
        "Connectivity, PhaseRatio, BoxCDim, Granul1, Granul2, Granul3, Granul4, Granul5, Erosion, Dilation, "\
        "Closure, Blur3"

    for filename in sys.argv[1:]:
        pil_img = Image.open(filename)
        image = misc.fromimage(pil_img.convert("L"))

        isize = interface_size(image)

        # get the domain size of the normal phase
        # then invert the image to get the second domain size
        ads1, std1 = average_domain_size(image)
        inverted = (image < image.mean())
        ads2, std2 = average_domain_size(inverted)

        avg_domain = (ads1 + ads2) / 2.0
        penalty = abs(ads1 - ads2) / avg_domain

        # fraction of phase one
        phase1 = np.count_nonzero(image)
        fraction = float(phase1) / float(image.size)

        # transfer distances
        # connectivity
        td, connectivity = transfer_distance(image)

        # granulometry data
        g_list = granulometry(image, range(1, 6))

        # Fractal dimension by box counting -- get the slope of the log/log fit
        bcd = box_counting_dimension(image)[0]*-1.0

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

        formatted = "%s %8.4f %8.4f %8.4f %8.4f %d %8.4f %8.4f %8.4f %8.4f %d %d %d %d %d %d %d %d %d"
        print formatted % (filename, ads1, std1, ads2, std2, isize, td, connectivity, fraction, bcd, \
            g_list[0], g_list[1], g_list[2], g_list[3], g_list[4], erosion_spots, dilation_spots, closure_spots, nzBlur)
