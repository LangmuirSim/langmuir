# -*- coding: utf-8 -*-
"""
curves.py
=========

.. argparse::
    :module: curves
    :func: create_parser
    :prog: curves.py

.. moduleauthor:: Geoff Hutchison <geoffh@pitt.edu>
"""
from PIL import Image
import argparse
import math
import os

desc = """
make sine curves (for fractal roughening)
"""

def create_parser():
    parser = argparse.ArgumentParser()
    parser.description = desc

    parser.add_argument(dest='width', default=256, type=int, nargs='?',
        metavar='dim.x', help='dim.x')

    parser.add_argument(dest='height', default=256, type=int, nargs='?',
        metavar='dim.y', help='dim.y')

    parser.add_argument(dest='spacing', default=16, type=int, nargs='?',
        metavar='spacing', help='spacing between waves, in pixels')

    parser.add_argument(dest='amplitude', default=4, type=int, nargs='?',
        metavar='amplitude', help='amplitude of the waves, in pixels')

    parser.add_argument(dest='period', default=8, type=int, nargs='?',
        metavar='period', help='number of waves along the image')

    parser.add_argument(dest='ofile', default=None, type=str, nargs='?',
        metavar='output', help='output file')

    return parser

def get_arguments(args=None):
    parser = create_parser()
    opts = parser.parse_args(args)
    return opts

def makeCurves(image, spacing = 16, amplitude = 4, periods = 8):
    (width, height) = image.size
    wavelength = width / periods
    copies = height / spacing + 2

    for copy in range(copies):
        for x in range(width):
            y = int(copy * spacing +
                amplitude * math.sin(math.pi * x / wavelength))
            if y < 0 or y > (height - 1):
                continue
            image.putpixel((x,y), 255)

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    # create a black and white image
    image = Image.new("L", (opts.width, opts.height))

    if opts.ofile is None:
        opts.ofile = "Curves-%d-%d-%d.png" % (
            opts.spacing, opts.amplitude, opts.period)

    makeCurves(image, opts.spacing, opts.amplitude, opts.period)
    image.save(opts.ofile, "PNG")
