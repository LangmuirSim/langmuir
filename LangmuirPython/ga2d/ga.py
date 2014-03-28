# -*- coding: utf-8 -*-
"""
ga.py
=====

.. argparse::
    :module: ga
    :func: create_parser
    :prog: ga.py

.. moduleauthor:: Geoff Hutchison <geoffh@pitt.edu>
"""
import argparse
import heapq
import random
import glob
import os
from multiprocessing import Pool
import time
import math

from PIL import Image
from scipy import ndimage, misc
import numpy as np


# our code
import ga_analyze as analyze
import modify
import grow

desc = """
Genetic algorithm design of 2D morphologies.
"""

def create_parser():
    parser = argparse.ArgumentParser()
    parser.description = desc

    parser.add_argument(dest='dir', type=str, metavar='directory',
        help='initial directory')

    parser.add_argument(dest='population', default=32, type=int, nargs='?',
        metavar='population', help='size of population')

    parser.add_argument(dest='generations', default=16, type=int, nargs='?',
        metavar='generations', help='# of generations')

    parser.add_argument(dest='children', default=10, type=int, nargs='?',
        metavar='children', help='# of children to create each generation')

    parser.add_argument(dest='mutability', default=8, type=int, nargs='?',
        metavar='mutability', help='# of items to mutate each generation')

    return parser

def get_arguments(args=None):
    parser = create_parser()
    opts = parser.parse_args(args)
    return opts

def score(filename):
    """
    .. todo:: comment function
    
    :param filename: image file name
    :type filename: str

    :return float: score (ideally as an estimated maximum power in W/(m^2))
    """
    # TODO: Make this physically and empirically based!

    # this works around a weird bug in scipy.misc.imread with 1-bit images
    pil_img = Image.open(filename)
    image = misc.fromimage(pil_img.convert("L"))

    width, height = image.shape
    if width != 256 or height != 256:
        print "Size Error: ", filename

    isize = analyze.interface_size(image)
    ads1, std1 = analyze.average_domain_size(image)

    # we now need to invert the image to get the second domain size
    inverted = (image < image.mean())
    ads2, std2 = analyze.average_domain_size(inverted)

    ads = (ads1 + ads2) / 2.0

    # penalize if there's a huge difference between the two domain sizes
    #penalty = abs(ads1 - ads2) / avg_domain

    # transfer distances
    # connectivity
    td, connectivity = analyze.transfer_distance(image)

    spots = np.logical_xor( image, ndimage.binary_erosion(image, structure=np.ones((2,2))) )
    erosion = np.count_nonzero(spots)
    spots = np.logical_xor( image, ndimage.binary_dilation(image, structure=np.ones((2,2))) )
    dilation = np.count_nonzero(spots)

    # fraction of phase one
    nonzero = np.count_nonzero(image)
    fraction = float(nonzero) / float(image.size)
    phase_scale = 4*fraction*(1.0-fraction)

    # from initial 74 simulations with nonlinear regression
    score= 3790.73 -41.867*ads -7.32114*td -0.191631*erosion + 788.541*math.tanh(5.83*(connectivity - 0.698)) \
           + 1.51379e-5*dilation**2
    return phase_scale**2 * score
    #return 2769.2 + 14490.3*pow(ads1,-1.83108) -19177.8*pow(ads1,-2.10393) -3658.22*pow(ads2,-32.9136)\
    #      -10.4209*pow(ads2,1.06774) -0.676183*td -0.00012812*pow(connectivity,-14.8147)

def score_filenames(filenames, pool=None):
    """
    .. todo:: comment function
    
    :param filenames: image file names
    :param pool: multiprocessing thread pool

    :type filenames: list of str
    :type pool: ?
    """    
    # analyze all these files and push into a list of (score, filename) tuples
    start = time.time()

    if pool is None:
        scores = map(score, filenames)
    else:
        scores = pool.map(score, filenames)
    print "Scoring time required: ", time.time() - start
    return zip(scores, filenames)

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()
    pool = Pool()

    if opts.dir[-1] != '/':
        opts.dir += '/'

    # glob through the directories for PNG files
    filenames = []
    for handle in glob.iglob(opts.dir + "/*.png"):
        filenames.append(handle)
    for handle in glob.iglob(opts.dir + "*/*.png"):
        filenames.append(handle)

    print "Initial Filenames: ", len(filenames)

    # check for a cache
    scores = []
    if os.path.exists('score.cache'):
        with open('score.cache') as cache:
            for line in cache:
                fileScore, filename = line.split()
                scores.append( (float(fileScore), filename) )

    # if needed, score the initial population
    if len(filenames) != len(scores):
        print "Re-scoring Initial Files"
        scores = score_filenames(filenames, pool)
        print "Caching initial scores"
        with open('score.cache', 'w') as cache:
            for fileScore, handle in scores:
                cache.write("%8.3f %s\n" % (fileScore, handle))

    # turn it into a (small) heap to easily remove the low scores
    heapq.heapify(scores)
    while len(scores) > opts.population:
        heapq.heappop(scores)
    # ok, now only the best (maximum) X items in scores remain
    print "Top Score: ", heapq.nlargest(1, scores)
    print "Worst Score: ", heapq.nsmallest(1, scores)

    # Here's the real GA code
    for generation in range(opts.generations):
        print "Starting Generation ", generation
        # create the directory for this new generation
        path = opts.dir + "gen-%d" % generation
        try:
            os.mkdir(path)
        except OSError:
            pass # don't worry if the directories exist

        # remove the worst X performers (i.e., to be replaced by children)
        for item in range(opts.children):
            heapq.heappop(scores)

        # first create the children
        parents = heapq.nlargest(opts.children * 2, scores)
        parentFiles = []
        for fileScore, handle in parents:
            parentFiles.append(handle)

        population = 0
        filenames = []
        print "Creating Children"
        for i in range(opts.children):
            # blend and threshold the images
            # all this code uses PIL not scipy.misc
            image1 = misc.imread(random.choice(parentFiles))
            image2 = misc.imread(random.choice(parentFiles))
            child = modify.blend_and_threshold(image1, image2)
            # save this new child
            childName = "%s/%d.png" % (path, population)
            filenames.append(childName)
            population += 1
            misc.imsave(childName, child)

        # make a copy of the files to be re-scored
        rescore_list = list(filenames)

        # Copy over the parents to the new generation (for mutation)
        # keep track of their old scores.. if they don't mutate
        #  .. we'll re-use it
        oldScores = []
        oldFiles = []
        while len(scores):
            fileScore, handle = heapq.heappop(scores)
            # copy the existing files before mutations
            image = misc.imread(handle)
            newName = "%s/%d.png" % (path, population)
            filenames.append(newName)
            # save the score and the new name
            oldScores.append(fileScore)
            oldFiles.append(newName)
            population += 1
            misc.imsave(newName, image)

        # TODO: Add more possible mutations
        print "Mutating"
        for i in range(opts.mutability):
            handle = random.choice(filenames)

            # if this is in oldscores, remove it.. need to rescore
            try:
                index = oldFiles.index(handle)
                del oldFiles[index]
                del oldScores[index]
            except ValueError:
                pass
            # add this to the list to rescore
            if handle not in rescore_list:
                rescore_list.append(handle)

            # this works around a weird bug in scipy with 1-bit images
            pil = Image.open(handle)
            image = misc.fromimage(pil.convert("L"))

            mutation = random.randrange(8)
            if mutation == 0:
                # gaussian blur
                image = modify.gblur_and_threshold(image, random.choice([2,3,4,5]))
            elif mutation == 1:
                # uniform blur
                image = modify.ublur_and_threshold(image, random.choice([2,3,4,5]))
            elif mutation == 2:
                # invert
                inverted = (image < image.mean())
                image = inverted
            elif mutation == 3:
                # add image-wide noise
                noise = np.random.random( image.shape )
                child = modify.blend_and_threshold(image, noise)
                image = child
            elif mutation == 4:
                # grow edges up to 50:50 mix
                nonzero = np.count_nonzero(image)
                phase = 0
                if nonzero > image.size / 2:
                    phase = 255
                child = grow.grow_ndimage(image, phase)
                image = child
            elif mutation == 5:
                # roughen the edges
                image = modify.roughen(image)
            elif mutation == 6:
                # rescale larger
                image = modify.enlarge(image)
            elif mutation == 7:
                # rescale smaller
                image = modify.shrink(image)
            # shift? - not useful
            # swap slices? - not sure if this is useful
            misc.imsave(handle, image)

        # create a new batch of scores
        print "Scoring Generation ", generation
        scores = score_filenames(rescore_list, pool)
        for i in range(len(oldFiles)):
            scores.append( (oldScores[i], oldFiles[i]) )
        heapq.heapify(scores)
        print "Top Score: ", heapq.nlargest(1, scores)
        print "Worst Score: ", heapq.nsmallest(1, scores)
