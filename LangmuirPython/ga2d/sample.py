# -*- coding: utf-8 -*-
from subprocess import check_call
from PIL import Image
import scipy.misc
import argparse
import modify
import os

def call(command, **kwargs):
    command = command.format(**kwargs)
    print command
    check_call(command.strip().split())

def morph(oname, func, **kwargs):
    out = func(image, **kwargs)
    oname = oname.format(stub=stub, **kwargs)
    scipy.misc.imsave(oname, out)

parser = argparse.ArgumentParser()
parser.add_argument(dest='fname')
parser.add_argument('--grow', action='store_true', help='run grow (slow)')
opts = parser.parse_args()

fname = opts.fname
stub, ext = os.path.splitext(fname)
image = scipy.misc.fromimage(Image.open(opts.fname).convert("L"))

script = './generators/block.py'
command = 'python {script} {fname} {stub}_blocked_L0_R1 --left 0 --right 1'
call(**locals())

morph('{stub}_gblur.png', modify.gblur_and_threshold, radius=16)

morph('{stub}_ublur.png', modify.ublur_and_threshold, radius=16)

morph('{stub}_shrink.png', modify.shrink, scale_x=0.5, scale_y=1.0)

morph('{stub}_enlarge.png', modify.enlarge, zoom_x=0.75, zoom_y=0.75)

morph('{stub}_pepper.png', modify.pepper, phase=0, count=0, percent=0.05,
      brush='point', overlap=False, maxTries=1e9)

morph('{stub}_roughen.png', modify.roughen, fraction=0.5, dilation=0, struct=None)

if opts.grow:
    morph('{stub}_grow.png', modify.grow_ndimage, phase=-1, pixToAdd=-1)

morph('{stub}_noise.png', modify.add_noise)

morph('{stub}_invert.png', modify.invert)
