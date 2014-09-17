# -*- coding: utf-8 -*-
"""
block.py
========

.. argparse::
    :module: block
    :func: create_parser
    :prog: block.py

.. moduleauthor:: Adam Gagorik <adam.gagorik@gmail.com>
"""
import argparse
import sys
import os

from scipy import misc, ndimage
import numpy as np
from PIL import Image

desc = """
Add blocking layer(s) to an existing image.

Use --left <pixel value> and/or --right <pixel value> to paint each side.
The --size <pixels> controls the width of the blocking layer.

By default, the blocking layer is painted up the entire image.
However, you can control the y-values of where the layer starts and stops with --lyi, --lyf, --ryi, and --ryf.
Please note that the image origin is in the upper left corner.
You may use negative indexing.  However, make sure the equivalent (positive) final pixel index is larger than the
equivalent (positive) initial pixel index, or nothing with be painted.
"""

def create_parser():
    parser = argparse.ArgumentParser()
    parser.description = desc

    parser.add_argument(dest='ifile', type=str, metavar='ifile.png',
        help='input')

    parser.add_argument(dest='ofile', type=str, metavar='ofile.png', default=None, nargs='?',
        help='output')

    parser.add_argument('--size', type=int, metavar='pixels', default=16,
        help='size of blocking layer(s)')

    parser.add_argument('--left', type=int, metavar='color', default=None, choices=[0, 1, 255],
        help='pixel value (0, 1 == 255) of left side')

    parser.add_argument('--right', type=int, metavar='color', default=None, choices=[0, 1, 255],
        help='pixel value (0, 1 == 255) of right side')

    parser.add_argument('--lyi', type=int, default=None, metavar='int',
        help='y-index to start painting at on left side')

    parser.add_argument('--lyf', type=int, default=None, metavar='int',
        help='y-index to stop painting at on left side')

    parser.add_argument('--ryi', type=int, default=None, metavar='int',
        help='y-index to start painting at on right side')

    parser.add_argument('--ryf', type=int, default=None, metavar='int',
        help='y-index to stop painting at on right side')

    parser.add_argument('--show', action='store_true', help='instead of saving, show result in a matplotlib window')

    return parser

def get_arguments(args=None):
    parser = create_parser()
    opts = parser.parse_args(args)

    # fix input file name
    if not os.path.exists(opts.ifile):
        parser.print_help()
        print >> sys.stderr, '\nfile does not exist: %s' % opts.ifile1
        sys.exit(-1)
    else:
        opts.ifile = os.path.abspath(opts.ifile)

    # make sure we are actually doing something
    if opts.right is None and opts.left is None:
        parser.print_help()
        print >> sys.stderr, '\nmust use --left # and/or --right #'
        sys.exit(-1)

    # fix right pixel value
    if not opts.right is None:
        if opts.right >= 1:
            opts.right = 255

    # fix left pixel value
    if not opts.left is None:
        if opts.left >= 1:
            opts.left = 255

    # fix output file name
    if opts.ofile is None or os.path.isdir(opts.ofile):
        stub, ext = os.path.splitext(opts.ifile)

        if os.path.isdir(opts.ofile):
            stub = os.path.join(os.path.abspath(opts.ofile), os.path.basename(stub))

        if opts.right is None:
            R = ''
        else:
            if opts.right >= 1:
                R = '_R1'
            else:
                R = '_R0'

            if not opts.ryi is None and not opts.ryf is None:
                R += '_%d:%d' % (opts.ryi, opts.ryf)
            else:
                if not opts.lyi is None:
                    R += '_%d:' % opts.ryi

                if not opts.lyf is None:
                    R += '_:%d' % opts.ryf

        if opts.left is None:
            L = ''
        else:
            if opts.left >= 1:
                L = '_L1'
            else:
                L = '_L0'

            if not opts.lyi is None and not opts.lyf is None:
                L += '_%d:%d' % (opts.lyi, opts.lyf)
            else:
                if not opts.lyi is None:
                    L += '_%d:' % opts.lyi

                if not opts.lyf is None:
                    L += '_:%d' % opts.lyf

        opts.ofile = '{stub}_block{L}{R}{ext}'.format(stub=stub, L=L, R=R, ext=ext)

    return opts

def makeBlock(image, left=None, right=None, size=16, lyi=None, lyf=None, ryi=None, ryf=None):
    """
    Paint blocking layer(s) over image on left and right side.

    :param image: data
    :param left: pixel value to paint on left side (None, 0, 1=255)
    :param right: pixel value to paint on right side (None, 0, 1=255)
    :param size: width of blocking layer (x-direction)

    :type image: :py:class:`numpy.ndarray`
    :type left: int
    :type right: int
    :type size: int

    :return: modified image data
    :rtype: :py:class:`numpy.ndarray`
    """
    # check parameters
    h, w = image.shape

    if not size <= w:
        raise ValueError("size of blocking layer (%d) larger than image width (%d)" % (opts.size, w))

    # aquire phase values
    max_pixel = np.amax(image)
    min_pixel = np.amin(image)

    # block right side
    if not right is None:
        if right > 0:
            right = max_pixel
        else:
            right = min_pixel

        image[ryi:ryf,:+size] = right

    # block left side
    if not left is None:
        if left > 0:
            left = max_pixel
        else:
            left = min_pixel

        image[lyi:lyf,-size:] = left

    return image

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    # open image
    image = Image.open(opts.ifile)
    image = misc.fromimage(image.convert("L"))

    # make image binary
    image[image >= 127.5] = 255
    image[image <= 127.5] = 0

    # apply blocking
    image = makeBlock(image, left=opts.left, right=opts.right, size=opts.size,
        lyi=opts.lyi, lyf=opts.lyf, ryi=opts.ryi, ryf=opts.ryf)

    # show image
    if opts.show:
        import matplotlib.pyplot as plt
        plt.autumn()
        plt.imshow(image)
        plt.tick_params(label1On=False, tick1On=False, tick2On=False)
        plt.tight_layout()
        plt.show()

    # save image
    else:
        misc.imsave(opts.ofile, image)
        print 'saved: {path}'.format(path=os.path.relpath(opts.ofile, work))