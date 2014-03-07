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
diffusion limited aggregation fractal images
"""

dimX = 256
dimY = 256
# create a black and white image
image = Image.new("L", (dimX, dimY))

sticky = 1.0
if len(sys.argv) > 1:
    sticky = float(sys.argv[1])

filename = "DLA-%5.3f-%d.png" % (sticky, random.randrange(1024))
print filename

# neighbor pixel directions
nx = [-1, 1,  0, 0, -1, 1,  0, 0, -1, 1,  0, 0, -1 ]
ny = [ 0, 0, -1, 1,  0, 0, -1, 1,  0, 0, -1, 1,  0 ]
#nx = [-1, -1, 0, 1, 1, 1, 0, -1]
#ny = [0, 1, 1, 1, 0, -1, -1, -1]
nbrs = len(nx)

particles = 0
#maxParticles = dimX * dimY / 3
maxParticles = 19500

initialX = dimX / 16 # updated as we launch more particles

while particles < maxParticles:
    # find an initial site
    x = initialX
    y = random.randrange(dimY)
    while image.getpixel((x,y)) > 0:
        # this site is occupied, find another one
        if initialX == (dimX - 1):
            x = random.randint(dimX / 2, dimX - 1)
        y = random.randrange(dimY)

    stuck = False
    stepCount = 0
    while not stuck and stepCount < dimX * 4:
        # is this site empty?
        if image.getpixel((x, y)) == 0:
            # check the neighbors
            for k in range(nbrs):
                xn = x + nx[k]
                yn = y + ny[k]

                if xn > (dimX - 1) or yn < 0 or yn > (dimY - 1):
                    continue
                if xn < 0:
                    image.putpixel((x, y), 255)
                    stuck = True
                    break
                if image.getpixel((xn, yn)) > 0 and random.random() < sticky:
                    image.putpixel((x, y), 255)
                    stuck = True
                    break

        # Pick a random (but acceptable) direction to move
        if not stuck:
            stepCount += 1
            # Check if we've wandered too far
            if x > initialX + 10:
                break

            accept = False
            tries = 0
            while not accept and tries < 8:
                a = random.randrange(nbrs)
                xn = x + nx[a]
                yn = y + ny[a]
                if xn < 0 or xn > (dimX - 1) or yn < 0 or yn > (dimX - 1):
                    continue

                if image.getpixel((xn,yn)) == 0:
                    x = xn
                    y = yn
                    accept = True
            # if we can't find a new place to move (e.g., neighbors are stuck)
            # we're stuck
            if tries == 8 and not accept:
                # start again
                stepCount = dimX

    # stuck another particle
    if stuck:
        particles += 1

        # check neighborhood for surrounded voids and fill them
        for k in range(nbrs):
            xn = x + nx[k]
            yn = y + ny[k]

            if xn < 0 or xn > (dimX - 1) or yn < 0 or yn > (dimY - 1):
                continue

            # for each of these sites we check if all its neighbors are filled
            allFilled = True
            for j in range(nbrs):
                xn2 = xn + nx[j]
                yn2 = yn + ny[j]

                if xn2 < 0 or xn2 > (dimX - 1) or yn2 < 0 or yn2 > (dimY - 1):
                    continue
                if image.getpixel((xn2, yn2)) == 0:
                    allFilled = False
                    break
            if allFilled:
                # this neighbor is surrounded, so fill it
                particles += 1
                image.putpixel((xn, yn), 255)

        if particles % 100 == 0:
            print particles
            image.save(filename, "PNG")

        # update where we launch the particles
        if x > initialX:
            initialX = x + 10
            if initialX >= dimX:
                initialX = dimX - 1

# PIL.ImageOps.invert(image)
image.save(filename, "PNG")
