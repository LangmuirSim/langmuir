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

desc = """
Genetic algorithm design of 2D morphologies.
""".lstrip()

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
    # TODO: Make this physically and empirically based!
    image = misc.imread(filename)

    isize = analyze.interface_size(image)
    ads1 = analyze.average_domain_size(image)

    # we now need to invert the image to get the second domain size
    inverted = (image < image.mean()).astype(np.uint8)
    ads2 = analyze.average_domain_size(inverted)

    avg_domain = (ads1 + ads2) / 2.0

    # penalize if there's a huge difference between the two domain sizes
    penalty = abs(ads1 - ads2) / avg_domain
    # TODO : use the penalty

    # we're arbitrarily claiming 13 nm as the best domain size
    domain_score = 10.0 - ((avg_domain - 13.0)**2)/15.0

    # transfer distances
    # connectivity
    td, connectivity = analyze.transfer_distance(image)

    value = float(domain_score) * float(isize * connectivity / td)
    if math.isnan(value):
        value = 0.0
    return value

def score_filenames(filenames, pool=None):
    # analyze all these files and push into a list of (score, filename) tuples
    start = time.time()
#    scores = forkmap.map(score, filenames)
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

    # score the initial population
    print "Scoring Initial Files"
    scores = score_filenames(filenames, pool)
    heapq.heapify(scores)

    # print the initial scores
    for fileScore, file in scores:
        print fileScore, file

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

        # Copy over the parents to the new generation (for mutation)
        while len(scores):
            fileScore, file = heapq.heappop(scores)
            # copy the existing files before mutations
            image = misc.imread(file)
            newName = "%s/%d.png" % (path, population)
            filenames.append(newName)
            population += 1
            misc.imsave(newName, image)

        # TODO: Add more possible mutations
        print "Mutating"
        for i in range(opts.mutability):
            file = random.choice(filenames)
            image = misc.imread(file)

            mutation = random.randrange(2)
            if mutation == 0:
                image = modify.gblur_and_threshold(image, random.choice([1,2,3,4,5]))
            elif mutation == 1:
                image = modify.ublur_and_threshold(image, random.choice([1,2,3,4,5]))
            elif mutation == 2:
                inverted = (image < image.mean())
                image = inverted
            # sharpen?
            # grow (edges)
            # add noise
            # shift
            # swap slices
            # rescale
            #   shrink and tile
            #   grow x and/or y

            misc.imsave(file, image)

        # create a new batch of scores
        print "Scoring Generation ", generation
        scores = score_filenames(filenames, pool)
        heapq.heapify(scores)
        print "Top Score: ", heapq.nlargest(1, scores)
        print "Worst Score: ", heapq.nsmallest(1, scores)
