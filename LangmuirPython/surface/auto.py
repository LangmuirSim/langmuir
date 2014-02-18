# -*- coding: utf-8 -*-
"""
@author: adam
"""
import scipy.signal as signal
import langmuir as lm
import numpy as np
import argparse
import sys
import os

desc = """
Perform Autocorrelation on surface or KPFM image.
"""

def get_arguments(args=None):
    parser = argparse.ArgumentParser()
    parser.description = desc
    parser.add_argument(dest='ifile', default='image.npy', type=str,
        nargs='?', metavar='input', help='input file')
    parser.add_argument('--stub', default='', type=str, metavar='stub',
        help='output file stub')
    parser.add_argument('--mode', default='correlate', type=str,
        choices=['correlate', 'convolve'], help='conv or corr')
    opts = parser.parse_args(args)
    if not os.path.exists(opts.ifile):
        parser.print_help()
        print >> sys.stderr, '\nfile does not exist: %s' % opts.ifile
        sys.exit(-1)
    return opts

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    image = lm.surface.load(opts.ifile)
    grid  = lm.grid.Grid(*image.shape)
    print grid

    print 'convoluting, it takes time...'
    if opts.mode == 'convolve':
        data = signal.convolve(image, image, mode='same')
        name = 'conv'

    if opts.mode == 'correlate':
        data = signal.correlate(image, image, mode='same')
        name = 'corr'

    handle = lm.common.format_output(stub=opts.stub, name=name, ext='npy')
    np.save(handle, data)
