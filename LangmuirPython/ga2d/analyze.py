# -*- coding: utf-8 -*-
"""
@author: Geoff Hutchison
"""
import argparse
import os
import sys

from scipy import ndimage, misc
import numpy as np
from numbapro import jit

from collections import deque

desc = """
analysis of images
"""

def get_arguments(args=None):
    parser = argparse.ArgumentParser()
    parser.description = desc

    parser.add_argument(dest='ifile', type=str, metavar='input',
        help='input file')

    opts = parser.parse_args()

    if not os.path.exists(opts.ifile):
        parser.print_help()
        print >> sys.stderr, '\nfile does not exist: %s' % opts.ifile
        sys.exit(-1)

    return opts

# from http://scipy-lectures.github.io/advanced/image_processing/
def disk_structure(n):
    struct = np.zeros((2 * n + 1, 2 * n + 1))
    x, y = np.indices((2 * n + 1, 2 * n + 1))
    mask = (x - n)**2 + (y - n)**2 <= n**2
    struct[mask] = 1
    return struct.astype(np.bool)

# from http://scipy-lectures.github.io/advanced/image_processing/
def granulometry(data, sizes=None):
    s = max(data.shape)
    if sizes == None:
        sizes = range(1, s/2, 2)
    granulo = [ndimage.binary_opening(data,
               structure=disk_structure(n)).sum() for n in sizes]
    return granulo

def average_domain_size(image):
    # get the average size of domains (i.e., distance from center of domain to other phase)
    # first use ndimage to label each domain
    labels, nb_labels = ndimage.label(image)

    # print "Number of domains: ", nb_labels

    # get the Euclidian distance to the other phase
    dists = ndimage.distance_transform_edt(image)
    # for each domain, get the maximum distance (i.e., the size of that domain)
    max = ndimage.measurements.maximum(dists, labels, index=np.arange(1, nb_labels + 1))
    # return the mean of the max array
    return np.mean(max)

def interface_size(image):
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
    work = os.getcwd()
    opts = get_arguments()

    image = misc.imread(opts.ifile)

    isize = interface_size(image)

    # get the domain size of the normal phase
    # then invert the image to get the second domain size
    ads1 = average_domain_size(image)
    inverted = (image < image.mean()).astype(np.uint8)
    ads2 = average_domain_size(inverted)

    # transfer distances
    # connectivity
    td, connectivity = transfer_distance(image)
    print ads1, ads2, isize, td, connectivity
