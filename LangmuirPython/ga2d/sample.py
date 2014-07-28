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
    image = scipy.misc.fromimage(Image.open(opts.fname).convert("L"))
    out = func(image, **kwargs)
    oname = oname.format(stub=stub, **kwargs)
    scipy.misc.imsave(oname, out)

parser = argparse.ArgumentParser()
parser.add_argument(dest='fname')
parser.add_argument('--grow', action='store_true', help='run grow (slow)')
opts = parser.parse_args(['tf5.png'])

fname = opts.fname
stub, ext = os.path.splitext(fname)

script = './generators/block.py'
command = 'python {script} {fname} {stub}_blocked_L0_R1.png --left 0 --right 1'
call(**locals())

command = 'python {script} {fname} {stub}_blocked_L1_R0.png --left 1 --right 0'
call(**locals())

for r in [2, 4, 6, 8, 10]:
    morph('{stub}_gblur_radius_{radius}.png', modify.gblur_and_threshold,
          radius=r)

for r in [8, 16, 24, 32]:
    morph('{stub}_ublur_radius_{radius}.png', modify.ublur_and_threshold,
          radius=r)

for s in [0.25, 0.50]:
    morph('{stub}_shrink_scale_{scale_x}_{scale_y}.png', modify.shrink,
          scale_x=s, scale_y=s)

for z in [1.25, 1.50]:
    morph('{stub}_enlarge_zoom_{zoom_x}_{zoom_y}.png', modify.enlarge,
          zoom_x=z, zoom_y=z)

for p in [0.05, 0.10]:
    morph('{stub}_pepper_point_percent_{percent}.png', modify.pepper, phase=0,
          count=0, percent=p, brush='point', overlap=False, maxTries=1e9)

    morph('{stub}_pepper_square_percent_{percent}.png', modify.pepper, phase=0,
          count=0, percent=p, brush='square', overlap=False, maxTries=1e9)

for f in [0.25, 0.50]:
    morph('{stub}_roughen_fraction_{fraction}_dilation_{dilation}.png',
          modify.roughen, fraction=f, dilation=0, struct=None)

if opts.grow:
    morph('{stub}_grow.png', modify.grow_ndimage, phase=-1, pixToAdd=-1)

morph('{stub}_noise.png', modify.add_noise)

morph('{stub}_invert.png', modify.invert)
