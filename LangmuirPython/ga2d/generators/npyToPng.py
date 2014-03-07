# -*- coding: utf-8 -*-
"""
@author: Geoff Hutchison
"""

import scipy.misc
import numpy as np
import argparse
import os

desc = """
convert from *.npy numpy binary files to PNG
"""

def get_arguments(args=None):
    parser = argparse.ArgumentParser(description=desc)

    parser.add_argument(dest='ifile', default="None",
                        type=str, metavar='input',
                        help='input file')

    parser.add_argument(dest='ofile', default="None",
                        type=str, metavar='output', nargs='?',
                        help='output file')

    opts = parser.parse_args()

    if not os.path.exists(opts.ifile):
        parser.print_help()
        print >> sys.stderr, '\nfile does not exist: %s' % opts.ifile
        sys.exit(-1)

    return opts

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    # default to the basename of the input file + png
    if opts.ofile == "None":
        opts.ofile = os.path.splitext(opts.ifile)[0] + '.png'

    extension = os.path.splitext(opts.ifile)[1]

    if extension == '.npy':
        data = np.load(opts.ifile)
    elif extension == '.csv':
        data = np.loadtxt(opts.ifile, dtype=np.uint8, delimiter=',')

    # scale the binary to a greyscale value
    data = data * 255

    scipy.misc.imsave(opts.ofile, data)
