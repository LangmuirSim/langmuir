# -*- coding: utf-8 -*-
"""
@author: adam
"""
import scipy.signal as signal
import langmuir as lm
import numpy as np
import argparse
import os

desc = """
Convolute two data sets.
"""

def get_arguments(args=None):
    parser = argparse.ArgumentParser()
    parser.description = desc

    parser.add_argument(dest='inputA', type=str, metavar='input',
        help='input file 1')

    parser.add_argument(dest='inputB', type=str, default=None, nargs='?',
        metavar='input', help='input file 2')

    parser.add_argument('--stub', default='', type=str, metavar='stub',
        help='output file stub')

    parser.add_argument('--corr', action='store_true',
        help='perform correlation instead of convolution')

    parser.add_argument('--mode', type=str, default='same', metavar='str',
        choices=['full', 'valid', 'same'], help='see scipy.signal.convolve')

    parser.add_argument('--fmt', type=str, default='npy', metavar='str',
        choices=['npy', 'pkl', 'txt', 'dat', 'csv'], help='output format')

    opts = parser.parse_args(args)

    if opts.inputB is None:
        opts.inputB = opts.inputA

    return opts

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    imageA = lm.surface.load(opts.inputA)
    imageB = lm.surface.load(opts.inputB)

    if opts.corr:
        result = signal.correlate(imageA, imageB, mode=opts.mode)
        name   = 'correlation'
    else:
        result = signal.convolve(imageA, imageB, mode=opts.mode)
        name   = 'convolution'

    handle = lm.common.format_output(stub=opts.stub, name=name, ext='npy')
    print 'saved: %s' % handle
    np.save(handle, result)