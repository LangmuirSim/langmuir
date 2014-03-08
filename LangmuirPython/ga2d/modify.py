# -*- coding: utf-8 -*-
"""
modify.py
=========

.. argparse::
    :module: modify
    :func: create_parser
    :prog: modify.py

.. moduleauthor:: Geoff Hutchison <geoffh@pitt.edu>
"""
from scipy import misc, ndimage
import argparse
import os

desc = r"""
Mix two images and re-threshold.
""".lstrip()

def create_parser():
    parser = argparse.ArgumentParser()
    parser.description = desc

    parser.add_argument(dest='ifile1', type=str, metavar='input1',
        help='input file #1')

    parser.add_argument(dest='ifile2', type=str, metavar='input2',
        help='input file #2')

    return parser

def get_arguments(args=None):
    parser = create_parser()
    opts = parser.parse_args(args)
    return opts

def threshold(image):
    """
    .. todo:: comment function
    
    :param image: data
    :type image: :py:class:`numpy.ndarray`
    """        
    return (image > image.mean())

def blend_and_threshold(image1, image2, ratio = 0.5):
    """
    .. todo:: comment function
    
    :param image1: data1
    :param image2: data2
    :param ratio: mixing ratio

    :type image1: :py:class:`numpy.ndarray`
    :type image2: :py:class:`numpy.ndarray`
    :type ratio: float
    """
    mixed = (ratio * image1 + (1.0 - ratio)*image2)
    image = ndimage.uniform_filter(mixed, size=2)
    return threshold(image)

def gblur_and_threshold(image, radius):
    """
    .. todo:: comment function
    
    :param image: data
    :param radius: kernel radius

    :type image1: :py:class:`numpy.ndarray`
    :type radius: float
    """    
    output = ndimage.gaussian_filter(image, sigma=radius)
    return threshold(output)

def ublur_and_threshold(image, radius):
    """
    .. todo:: comment function
    
    :param image: data
    :param radius: kernel radius

    :type image1: :py:class:`numpy.ndarray`
    :type radius: float
    """        
    output = ndimage.uniform_filter(image, size=radius)
    return threshold(output)

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    image1 = misc.imread(opts.ifile1)
    image2 = misc.imread(opts.ifile2)

    output = blend_and_threshold(image1, image2)
    misc.imsave("Blend.png", output)
