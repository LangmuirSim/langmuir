# -*- coding: utf-8 -*-
"""
tristripe.py
============

.. argparse::
    :module: tristripe
    :func: create_parser
    :prog: tristripe.py

.. moduleauthor:: Geoff Hutchison <geoffh@pitt.edu>
"""
from PIL import Image
import argparse
import os

desc = """
make triangle stripes (horizontal triangles)
"""

def create_parser():
    parser = argparse.ArgumentParser()
    parser.description = desc

    parser.add_argument(dest='ofile', default=None, type=str, nargs='?',
        metavar='output', help='output file')

    parser.add_argument(dest='width', default=256, type=int, metavar='dim.x',
                        nargs='?', help='dim.x')

    parser.add_argument(dest='height', default=256, type=int, metavar='dim.y',
                        nargs='?', help='dim.y')

    parser.add_argument(dest='period', default=32, type=int, metavar='period',
                        nargs='?', help='number of triangles along the image')

    return parser

def get_arguments(args=None):
    parser = create_parser()
    opts = parser.parse_args(args)
    return opts

def makeTriStripes(image, periods = 8):
    (width, height) = image.size

    offset = height / periods
    half = offset / 2
    slope = float(half) / float(width)

    for copy in range(periods + 3):
        for x in range(width):
            for sY in range(offset):
                y = sY + copy*offset
                if y > height - 1:
                    break
                if sY > int(x * slope) and sY < int(offset - x*slope):
                    image.putpixel((x,y), 255)

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    # create a black and white image
    image = Image.new("L", (opts.width, opts.height))

    if opts.ofile is None:
        opts.ofile = "TriStripe-%d.png" % (opts.period)

    makeTriStripes(image, opts.period)
    image.save(opts.ofile, "PNG")
