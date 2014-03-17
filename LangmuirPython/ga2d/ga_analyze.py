# -*- coding: utf-8 -*-
"""
ga_analyze.py
=============

.. moduleauthor:: Geoff Hutchison <geoffh@pitt.edu>
"""
import sys
import argparse
from scipy import ndimage, misc
import numpy as np
from collections import deque

desc = """
Analysis of images for GA processing.
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
    .. todo:: comment function

    :param n: ?
    :type n: int
    """
    struct = np.zeros((2 * n + 1, 2 * n + 1))
    x, y = np.indices((2 * n + 1, 2 * n + 1))
    mask = (x - n)**2 + (y - n)**2 <= n**2
    struct[mask] = 1
    return struct.astype(np.bool)

def square_structure(n):
    """

    """
    struct = np.ones((n,n), dtype=np.bool)
    return struct

# from http://scipy-lectures.github.io/advanced/image_processing/
def granulometry(image, sizes=None, structure=disk_structure):
    """
    .. todo:: comment function

    :param image: ?
    :param sizes: ?

    :type data: :py:class:`numpy.ndarray`
    :type sizes: list[int]
    """
    s = max(image.shape)
    if sizes == None:
        sizes = range(1, s/2, 2)
    granulo = [ndimage.binary_opening(image,
               structure=structure(n)).sum() for n in sizes]
    return granulo

def average_domain_size(image):
    """
    .. todo:: comment function

    :param image: data
    :type image: :py:class:`numpy.ndarray`
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
    return np.mean(imax)

def box_counting_dimension(image):
    # use granulometry for
    dim = max(image.shape)
    sizes = [ 1, 2, 4, 8 ]
    grains = granulometry(image, sizes, structure=square_structure)

    return np.polyfit(np.log(sizes), np.log(grains), 1)

def interface_size(image):
    """
    .. todo:: comment function

    :param image: data
    :type image: :py:class:`numpy.ndarray`
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
    .. todo:: comment function

    :param original: data
    :type original: :py:class:`numpy.ndarray`
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
    # .. it still works similarly
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

    avgDistance = avgDistance / count
    connectivityFraction = 1.0 - float(unconnectedCount) / float(width * height)

    return avgDistance, connectivityFraction

if __name__ == '__main__':

    print "Filename, AvgDomainSize1, AvgDomainSize2, InterfaceSize, AvgTransferDist, Connectivity, PhaseRatio", \
        "Granul1, Granul2, Granul3, Granul4, Granul5"

    for filename in sys.argv[1:]:
        image = misc.imread(filename)

        isize = interface_size(image)

        # get the domain size of the normal phase
        # then invert the image to get the second domain size
        ads1 = average_domain_size(image)
        inverted = (image < image.mean())
        ads2 = average_domain_size(inverted)

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

        formatted = "%s %8.4f %8.4f %d %8.4f %8.4f %8.4f %d %d %d %d %d"
        print formatted % (filename, ads1, ads2, isize, td, connectivity, fraction, \
            g_list[0], g_list[1], g_list[2], g_list[3], g_list[4])
