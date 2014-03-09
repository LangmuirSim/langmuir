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
import scipy.misc as misc
import numpy as np
import argparse
import heapq
import random
import math
import glob
import os

from multiprocessing import Pool
import time

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
        metavar='children', help='# of items to mutate each generation')

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

    :return float: score (ideally as an estimated maximum power)
    """
    # TODO: Make this physically and empirically based!
    image = misc.imread(filename)

    width, height = image.shape
    if width != 256 or height != 256:
        print "Size Error: ", filename

    isize = analyze.interface_size(image)
    ads1 = analyze.average_domain_size(image)

    # we now need to invert the image to get the second domain size
    inverted = (image < image.mean())
    ads2 = analyze.average_domain_size(inverted)

    avg_domain = (ads1 + ads2) / 2.0

    # penalize if there's a huge difference between the two domain sizes
    #penalty = abs(ads1 - ads2) / avg_domain

    # transfer distances
    # connectivity
    td, connectivity = analyze.transfer_distance(image)

    # fraction of phase one
    nonzero = np.count_nonzero(image)
    fraction = float(nonzero) / float(image.size)
    scale = 4.0*fraction*(1.0 - fraction)

    return scale * (4497.2167 - 107.4394*ads1 + 85.3286*ads2 - 0.1855*isize - 10.9523*td + 169.7597*connectivity)

def score_filenames(filenames, pool=None):
    """
    .. todo:: comment function
    
    :param filenames: image file names
    :param pool: multiprocessing thread pool

    :type filename: list of str
    :type pool: ?
    """    
    # analyze all these files and push into a list of (score, filename) tuples
    start = time.time()

    if pool == None:
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
    for file in glob.iglob(opts.dir + "/*.png"):
        filenames.append(file)
    for file in glob.iglob(opts.dir + "*/*.png"):
        filenames.append(file)

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
            for fileScore, file in scores:
                cache.write("%8.3f %s\n" % (fileScore, file))

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
        for fileScore, file in parents:
            parentFiles.append(file)

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
            fileScore, file = heapq.heappop(scores)
            # copy the existing files before mutations
            image = misc.imread(file)
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
            file = random.choice(filenames)

            # if this is in oldscores, remove it.. need to rescore
            try:
                index = oldFiles.index(file)
                del oldFiles[index]
                del oldScores[index]
            except ValueError:
                pass
            # add this to the list to rescore
            if file not in rescore_list:
                rescore_list.append(file)

            image = misc.imread(file)

            mutation = random.randrange(6)
            if mutation == 0:
                # gaussian blur
                image = modify.gblur_and_threshold(image, random.choice([1,2,3,4,5]))
            elif mutation == 1:
                # uniform blur
                image = modify.ublur_and_threshold(image, random.choice([1,2,3,4,5]))
            elif mutation == 2:
                # invert
                inverted = (image < image.mean())
                image = inverted
            elif mutation == 3:
                # add noise
                noise = np.random.random( image.shape )
                child = modify.blend_and_threshold(image, noise)
                image = child
            elif mutation == 4:
                # grow edges
                nonzero = np.count_nonzero(image)
                phase = 0
                if nonzero > image.size / 2:
                    phase = 255
                child = grow.grow_ndimage(image, phase)
                image = child
            elif mutation == 5:
                # rescale larger
                image = modify.grow(image)
            elif mutation == 6:
                # rescale smaller
                image = modify.shrink(image)
            # shift?
            # swap slices?
            misc.imsave(file, image)

        # create a new batch of scores
        print "Scoring Generation ", generation
        scores = score_filenames(rescore_list, pool)
        for i in range(len(oldFiles)):
            scores.append( (oldScores[i], oldFiles[i]) )
        heapq.heapify(scores)
        print "Top Score: ", heapq.nlargest(1, scores)
        print "Worst Score: ", heapq.nsmallest(1, scores)
