# -*- coding: utf-8 -*-
"""
@author: Geoff Hutchison
"""

from PIL import Image
import random

desc = """
make simple lines (for fractal growing)
"""

def makeLines(image, thickness = 1, spacing = 16):
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

if __name__ == '__main__':
    dimX = 128
    dimY = 128
    # create a black and white image
    image = Image.new("L", (dimX, dimY))

    filename = "Lines-%d.png" % random.randrange(1024)

    makeLines(image)
    image.save(filename, "PNG")
