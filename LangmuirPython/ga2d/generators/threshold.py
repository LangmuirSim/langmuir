# -*- coding: utf-8 -*-
"""
block.py
========

.. argparse::
    :module: threshold
    :func: create_parser
    :prog: threshold.py

.. moduleauthor:: Adam Gagorik <adam.gagorik@gmail.com>
"""
from scipy import misc
from PIL import Image
import numpy as np
import argparse
import os
import re

desc = """
Threshold an image.
"""

def find_inputs(work, exts):
    """
    Search for input files.
    """
    found = []
    root, dirs, files = os.walk(os.getcwd()).next()
    for f in files:
        stub, ext = os.path.splitext(f)
        if ext.lower() in exts:
            if not re.match('.*_percentile_.*', f):
                if not re.match('.*_mean_.*', f):
                    found.append(os.path.join(root, f))
    return found

def create_parser():
    parser = argparse.ArgumentParser()
    parser.description = desc

    parser.add_argument(dest='images', nargs='*', help='input file names',
        type=str, default=[], metavar='image.png')
    
    parser.add_argument('--mode', choices=['mean', 'percentile'],
        help='thresholding mode')
    
    parser.add_argument('--mean', action='store_true',
        help='threshold using mean value')

    parser.add_argument('--percentile', default=None, type=float,
        help='threshold using percentile value')    

    parser.add_argument('--lmap', action='store_true',
        help='apply linear mapping to [0,1] before threshold')
    
    parser.add_argument('--show', action='store_true', help='show image')

    return parser

def get_arguments(args=None):
    parser = create_parser()
    opts = parser.parse_args(args)

    exts = ['.png', '.jpeg', '.jpg']

    # search for images if None were passed
    if not opts.images:
        opts.images = find_inputs(os.getcwd(), exts)

    # make sure images were found
    if not opts.images:
        raise RuntimeError('no images found! %s' % ' '.join(exts))
    else:
        print 'found %d images' % len(opts.images)

    # make sure absolute paths are used
    opts.images = [os.path.abspath(image) for image in opts.images]

    # make sure images exist
    for image in opts.images:
        if not os.path.exists(image):
            raise RuntimeError('image does not exist: %s' % image)
    
    if opts.show:
        if not len(opts.images) == 1:
            raise RuntimeError('can not use --show with multiple images!')

    if not opts.mean and opts.percentile is None:
        raise RuntimeError('must use --mean or --percentile')

    if opts.mean:
        if not opts.percentile is None:
            raise RuntimeError('can not choose both mean and percentile')

    if not opts.percentile is None:
        if not opts.percentile <= 1 and opts.percentile >= 0:
            raise RuntimeError('percentile must be between 0 and 1')

    return opts

def linear_mapping(array, n=0.0, m=1.0):
    """
    Map values into range.
    """
    array = np.asanyarray(array)
    a = np.amin(array)
    b = np.amax(array)
    return (array - a) / (b - a) * (m - n) + n

def threshold(a, v=None, v0=0, v1=255):
    """
    Threshold into binary image.
    """
    assert v1 > v0

    image = np.copy(a)

    if v is None:
        v = image.mean()

    image[image > v ] = v1
    image[image < v1] = v0

    return misc.fromimage(Image.fromarray(image).convert("L")).astype(np.uint8)

def imshow(image, **kwargs):
    """
    Show image.
    """
    import matplotlib.pyplot as plt
    plt.autumn()
    plt.imshow(image, **kwargs)
    plt.tick_params(label1On=False, tick1On=False, tick2On=False)
    plt.tight_layout()
    plt.show()

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    for fname in opts.images:
        image = misc.fromimage(Image.open(fname).convert("L"))
        
        if opts.lmap:
            image = linear_mapping(image, 0.0, 1.0)
        
        if opts.mean:
            image = threshold(image, np.mean(image))
        
        if not opts.percentile is None:
            image = threshold(image,
                np.percentile(image, 100 * opts.percentile))
        
        if opts.show:
            imshow(image)
        
        stub, ext = os.path.splitext(fname)
        
        if opts.mean:
            oname = '{stub}_mean{ext}'.format(stub=stub, ext=ext)
        
        if opts.percentile:
            oname = '{stub}_percentile_{value:.0f}{ext}'.format(stub=stub,
                ext=ext, value=opts.percentile * 100)
        
        misc.imsave(oname, image)