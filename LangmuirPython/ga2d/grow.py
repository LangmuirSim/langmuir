# -*- coding: utf-8 -*-
"""
@author: Geoff Hutchison
"""
import argparse
import os
import random
from PIL import Image
import sys

desc = """
append particles to an existing image
(adds fractal disorder to white boundaries)
"""

def get_arguments(args=None):
    parser = argparse.ArgumentParser()
    parser.description = desc

    parser.add_argument(dest='ifile', type=str, metavar='input',
        help='input file')

    opts = parser.parse_args()

    if not os.path.exists(opts.ifile):
        parser.print_help()
        print >> sys.stderr, '\nfile does not exist: %s' % opts.ifile
        sys.exit(-1)

    return opts

def growImage(image, pixToAdd = -1):
    # neighbor pixel directions
    nx = [-1, 1,  0, 0]
    ny = [ 0, 0, -1, 1]
    nbrs = len(nx)

    (width, height) = image.size

    if pixToAdd == -1:
        pixToAdd = width * height / 2

    # count how many are currently set
    pix = 0
    for x in range(width):
        for y in range(height):
            if image.getpixel((x, y)) > 0:
                pix += 1

    while pix < pixToAdd:
        x = random.randrange(width)
        y = random.randrange(height)
        if image.getpixel((x,y)) > 0:
            for i in range(nbrs):
                # x, y is set, so try to find an empty neighbor
                k = random.randrange(nbrs)
                xn = x + nx[k]
                yn = y + ny[k]
                if xn < 0 or xn > width - 1 or yn < 0 or yn > height - 1:
                    continue
                if image.getpixel((xn,yn)) == 0:
                    image.putpixel((xn,yn), 255)
                    pix += 1
                    break

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    image = Image.open(opts.ifile)
    growImage(image)
    image.save(opts.ifile, "PNG")
