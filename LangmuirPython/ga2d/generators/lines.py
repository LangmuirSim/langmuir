# -*- coding: utf-8 -*-
"""
lines.py
========

.. argparse::
    :module: lines
    :func: create_parser
    :prog: lines.py

.. moduleauthor:: Geoff Hutchison <geoffh@pitt.edu>
"""
from PIL import Image
import argparse
import os

desc = """
make simple lines (for fractal growing)
"""

def create_parser():
    parser = argparse.ArgumentParser()
    parser.description = desc

    parser.add_argument(dest='width', default=256, type=int, metavar='dim.x',
                        nargs='?', help='dim.x')

    parser.add_argument(dest='height', default=256, type=int, metavar='dim.y',
                        nargs='?', help='dim.y')

    parser.add_argument(dest='spacing', default=16, type=int, metavar='spacing',
                        nargs='?', help='spacing between waves, in pixels')

    parser.add_argument(dest='thickness', default=1, type=int, metavar='thickness',
                        nargs='?', help='thickness of the bands, in pixels')

    parser.add_argument(dest='ofile', default=None, type=str, nargs='?',
        metavar='output', help='output file')

    return parser

def get_arguments(args=None):
    parser = create_parser()
    opts = parser.parse_args(args)
    return opts

def makeLines(image, thickness = 1, spacing = 12):
    (width, height) = image.size

    on = False    # are we writing lines?
    lineCount = 0 # how many of the width have been written
    for y in range(height):
        if y % spacing == 0:
            on = True

        if on:
            for x in range(width):
                image.putpixel((x,y), 255)
            # wrote the line, see if we need more
            lineCount += 1
            if lineCount > thickness:
                on = False
                lineCount = 0

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    # create a black and white image
    image = Image.new("L", (opts.width, opts.height))

    if opts.ofile is None:
        opts.ofile = "Lines-%d.png" % opts.spacing

    makeLines(image, opts.thickness, opts.spacing)
    image.save(opts.ofile, "PNG")
