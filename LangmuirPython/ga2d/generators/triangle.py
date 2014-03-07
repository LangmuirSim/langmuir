# -*- coding: utf-8 -*-
"""
@author: Geoff Hutchison
"""

from PIL import Image
import random
import math
import argparse
import os

desc = """
make triangle waves (for fractal roughening)
"""

def get_arguments(args=None):
    parser = argparse.ArgumentParser()
    parser.description = desc

    parser.add_argument(dest='ofile', default=("Tri-%d.png" % random.randrange(1024)),
                        type=str, metavar='output',
                        help='output file')

    parser.add_argument(dest='width', default=128, type=int, metavar='dim.x',
                        help='dim.x')

    parser.add_argument(dest='height', default=128, type=int, metavar='dim.y',
                        help='dim.y')

    parser.add_argument(dest='spacing', default=16, type=int, metavar='spacing',
                        help='spacing between waves, in pixels')

    parser.add_argument(dest='amplitude', default=4, type=int, metavar='amplitude',
                        help='amplitude of the waves, in pixels')

    parser.add_argument(dest='period', default=8, type=int, metavar='period',
                        help='number of waves along the image')

    opts = parser.parse_args()

    return opts

def makeTriWaves(image, spacing = 16, amplitude = 4, periods = 8):
    (width, height) = image.size
    wavelength = width / periods
    copies = height / spacing + 2

    for copy in range(copies):
        for x in range(width):
            y = int(copy * spacing + 2 * amplitude * math.asin(math.sin(math.pi * x / wavelength)) / math.pi )
            if y < 0 or y > (height - 1):
                continue
            image.putpixel((x,y), 255)


if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    # create a black and white image
    image = Image.new("L", (opts.width, opts.height))

    makeTriWaves(image, opts.spacing, opts.amplitude, opts.period)
    image.save(opts.ofile, "PNG")
