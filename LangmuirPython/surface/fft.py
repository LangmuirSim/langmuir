# -*- coding: utf-8 -*-
"""
@author: adam
"""
import langmuir as lm
import collections
import argparse
import sys
import os

desc = """
Perform FFT on surface or KPFM image.
"""

def get_arguments(args=None):
    parser = argparse.ArgumentParser()
    parser.description = desc

    parser.add_argument(dest='ifile', type=str, metavar='input',
        help='input file')

    parser.add_argument('--stub', default='', type=str, metavar='stub',
        help='output file stub')

    parser.add_argument('--nodetrend', action='store_true',
        help='do not subtract average from signal')

    parser.add_argument('--window', action='store_true',
        help='multiply signal by hamming window')

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
    xsize = image.shape[0]
    ysize = image.shape[1]
    zsize = image.shape[2]
    grid  = lm.grid.Grid(xsize, ysize, zsize)
    fft   = lm.surface.FFT3D(grid.mx, grid.my, grid.mz, image,
        detrend=not opts.nodetrend, window=opts.window)

    results = collections.OrderedDict()
    results['x'] = fft.x
    results['y'] = fft.y
    results['z'] = fft.z
    results['s'] = fft.s
    results['u'] = fft.u
    results['v'] = fft.v
    results['w'] = fft.w
    results['f'] = fft.fft
    results['p'] = fft.power

    handle = lm.common.format_output(stub=opts.stub, name='fft', ext='pkl')
    lm.common.save_pkl(results, handle)
    print 'saved: %s' % handle.name