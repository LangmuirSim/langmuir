# -*- coding: utf-8 -*-
"""
npyToPng.py
===========

.. argparse::
    :module: npyToPng
    :func: create_parser
    :prog: npyToPng.py

.. moduleauthor:: Geoff Hutchison <geoffh@pitt.edu>
"""
import numpy as np
import scipy.misc
import argparse
import os

desc = """
convert from *.npy numpy binary files to PNG
"""

def create_parser():
    parser = argparse.ArgumentParser(description=desc)

    parser.add_argument(dest='ifile', type=str, metavar='input',
        help='input file')

    parser.add_argument(dest='ofile', default=None, type=str, nargs='?',
        metavar='output', help='output file')

    return parser

def get_arguments(args=None):
    parser = create_parser()
    opts = parser.parse_args(args)
    return opts

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    stub, ext = os.path.splitext(opts.ifile)

    # default to the basename of the input file + png
    if opts.ofile is None:
        opts.ofile = '%s.png' % stub

    if ext == '.npy':
        data = np.load(opts.ifile)
    elif ext == '.csv':
        data = np.loadtxt(opts.ifile, dtype=np.uint8, delimiter=',')

    # scale the binary to a greyscale value
    data *= 255

    scipy.misc.imsave(opts.ofile, data)
