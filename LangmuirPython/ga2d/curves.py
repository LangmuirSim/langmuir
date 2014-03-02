# -*- coding: utf-8 -*-
"""
@author: Geoff Hutchison
"""

from PIL import Image
import random
import math

desc = """
make sine curves (for fractal growing)
"""

def makeCurves(image, spacing = 16, amplitude = 4, periods = 8):
    (width, height) = image.size
    wavelength = width / periods
    copies = height / spacing + 2

    for copy in range(copies):
        for x in range(width):
            y = int(copy * spacing + amplitude * math.sin(math.pi * x / wavelength))
            if y < 0 or y > (height - 1):
                continue
            image.putpixel((x,y), 255)


if __name__ == '__main__':
    dimX = 128
    dimY = 128
    # create a black and white image
    image = Image.new("L", (dimX, dimY))

    filename = "Curves-%d.png" % random.randrange(1024)

    makeCurves(image)
    image.save(filename, "PNG")
