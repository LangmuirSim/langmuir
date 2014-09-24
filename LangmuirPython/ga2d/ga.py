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

    parser.add_argument(dest='diversity', default=0.02, type=float, nargs='?',
        metavar='diversity', help='% of pixels that need to differ')

    parser.add_argument('--rescore', action='store_true', help='rescore initial files')

    return parser

def get_arguments(args=None):
    parser = create_parser()
    opts = parser.parse_args(args)
    return opts

def score(filename):
    """
    Score individual image files for the genetic algorithm.
    The idea is to derive predictive factors for the langmuir performance
    (i.e., max power) based on the connectivity, phase fractions, domain sizes,
    etc. The scoring function should be based on multivariate fits from a database
    of existing simulations. To ensure good results, use robust regression techniques
    and cross-validate the best-fit.

    :param filename: image file name
    :type filename: str

    :return score (ideally as an estimated maximum power in W/(m^2))
    :rtype float
    """
    # this works around a weird bug in scipy.misc.imread with 1-bit images
    # open them with PIL as 8-bit greyscale "L" and then convert to ndimage
    pil_img = Image.open(filename)
    image = misc.fromimage(pil_img.convert("L"))

    width, height = image.shape
    if width != 256 or height != 256:
        print "Size Error: ", filename

    #    isize = analyze.interface_size(image)
    ads1, std1 = analyze.average_domain_size(image)

    # we now need to invert the image to get the second domain size
    inverted = (image < image.mean())
    ads2, std2 = analyze.average_domain_size(inverted)

    #overall average domain size
    ads = (ads1 + ads2) / 2.0

    # transfer distances
    # connectivity
    td1, connect1, td2, connect2 = analyze.transfer_distance(image)

    spots = np.logical_xor( image, ndimage.binary_erosion(image, structure=np.ones((2,2))) )
    erosion = np.count_nonzero(spots)
    spots = np.logical_xor( image, ndimage.binary_dilation(image, structure=np.ones((2,2))) )
    dilation = np.count_nonzero(spots)

    # fraction of phase one
    nonzero = np.count_nonzero(image)
    fraction = float(nonzero) / float(image.size)
    # scores zero at 0, 1 and maximum at 0.5
    ps = fraction*(1.0-fraction)

    # from simulations with multivariate nonlinear regression
    return (-1.98566e8) + (-1650.14)/ads + (-680.92)*math.pow(ads,0.25) + \
           1.56236e7*math.tanh(14.5*(connect1 + 0.4)) + 1.82945e8*math.tanh(14.5*(connect2 + 0.4)) \
           + 2231.32*connect1*connect2 \
           + (-4.72813)*td1 + (-4.86025)*td2 \
           + 3.79109e7*ps**8 \
           + 0.0540293*dilation + 0.0700451*erosion

def score_filenames(filenames, pool=None):
    """
    Score a list of files in parallel using the supplied processing pool

    :param filenames: image file names
    :param pool: multiprocessing thread pool

    :type filenames: list of str
    :type pool: multiprocessing.pool.Pool

    :return: list of tuples (score, filename)
    :rtype: list of tuple
    """
    # analyze all these files and push into a list of (score, filename) tuples
    start = time.time()

    if pool is None:
        scores = map(score, filenames)
    else:
        scores = pool.map(score, filenames)
    print "Scoring time required: ", time.time() - start
    return zip(scores, filenames)

def diversity_check(image1, image2):
    """
    Compare two

    :param file1: first filename
    :param file2: second filename
    :type file1: string
    :type file2: string

    :return: fraction of sites that differ
    """
    # If pixels are the same, XOR will return 0
    # .. so this counts number of different sites / number of sites
    return float(np.count_nonzero( np.logical_xor(image1, image2) )) / float(image1.size)


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
    if opts.rescore or len(filenames) != len(scores):
        print "Re-scoring Initial Files"
        scores = score_filenames(filenames, pool)
        print "Caching initial scores"
        with open('score.cache', 'w') as cache:
            for fileScore, handle in scores:
                cache.write("%8.3f %s\n" % (fileScore, handle))

    # ok, sort the list
    scores.sort(reverse=True)
    print "Top Score: ", scores[0]
    # We need to prune out non-diverse files
    i = 0
    while i < len(scores):
        (score1, name1) = scores[i]
        pil_img = Image.open(name1)
        image1 = misc.fromimage(pil_img.convert("L"))
        j = i+1
        while j < len(scores):
            (score2, name2) = scores[j]
            pil_img = Image.open(name2)
            image2 = misc.fromimage(pil_img.convert("L"))
            difference = diversity_check(image1, image2)
            if difference < opts.diversity:
                print "Removing ", (score2, name2), " with low diversity", difference
                scores.pop(j)
            else:
                j += 1
        i += 1
    # it's now filtered by diversity and sorted by score, so remove the worst scores
    while len(scores) > opts.population:
        scores.pop()
    print "Median Score: ", scores[len(scores)/2]
    print "Worst Score: ", scores[-1]

    # Finally, got the initial population and we can get to work
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
        heapq.heapify(scores)
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
        uniqueParents = []
        for i in range(opts.children):
            # blend and threshold the images
            # all this code uses PIL not scipy.misc

            # make sure we pick unique parents
            # .. and that each child has two different parents
            # We might still create the same child in different generations
            # but the diversity filter will get that later
            parent1 = random.choice(parentFiles)
            parent2 = random.choice(parentFiles)
            while parent2 == parent1 or (parent1, parent2) in uniqueParents:
                parent2 = random.choice(parentFiles)
            uniqueParents.append( (parent1, parent2) )

            image1 = misc.imread(parent1)
            image2 = misc.imread(parent2)

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
        #  .. we'll re-use it, rather than re-computing
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

            mutation = random.randrange(7)
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
                # roughen the edges
                image = modify.roughen(image)
            elif mutation == 5:
                # rescale larger
                image = modify.enlarge(image)
            elif mutation == 6:
                # rescale smaller
                image = modify.shrink(image)
            # disabled for now
            elif mutation == 7:
                # grow edges up to 50:50 mix
                nonzero = np.count_nonzero(image)
                phase = 0
                if nonzero > image.size / 2:
                    phase = 255
                child = modify.grow_ndimage(image, phase)
                image = child
            if image.shape != (256, 256):
                print "Shape error: ", handle, mutation
                # don't save this mutation, or we'll die later
                continue
            # shift? - not useful
            # swap slices? - not sure if this is useful
            misc.imsave(handle, image)

        # create a new batch of scores
        print "Scoring Generation ", generation
        scores = score_filenames(rescore_list, pool)
        for i in range(len(oldFiles)):
            scores.append( (oldScores[i], oldFiles[i]) )

        scores.sort(reverse=True)
        print "Top Score: ", scores[0]
        print "Median Score: ", scores[len(scores)/2]
        print "Worst Score: ", scores[-1]
        # We need to prune out non-diverse files
        # In this case, rather than  removing them entirely, we'll set the score to negative :-)
        i = 0
        while i < len(scores):
            (score1, name1) = scores[i]
            pil_img = Image.open(name1)
            image1 = misc.fromimage(pil_img.convert("L"))
            j = i+1
            while j < len(scores):
                (score2, name2) = scores[j]
                pil_img = Image.open(name2)
                image2 = misc.fromimage(pil_img.convert("L"))
                difference = diversity_check(image1, image2)
                if difference < opts.diversity:
                    score2 = -1.0 * math.fabs(score2)
                    scores[j] = (score2, name2)
                    # print " Inverting score: ", scores[j]
                j += 1
            i += 1
        scores.sort(reverse=True)
