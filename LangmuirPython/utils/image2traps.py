# -*- coding: utf-8 -*-
import PyQt4.QtCore as QtCore
import PyQt4.QtGui as QtGui
import langmuir as lm
import argparse

desc = "create input file from black and white image of traps"

def get_arguments(args=None):
    parser = argparse.ArgumentParser(description=desc)
    parser.add_argument(dest='image', help='name of image file',
                        metavar='image.png')
    parser.add_argument(dest='inp', default='sim.inp', nargs='?',
                        help='name of output file', metavar='sim.inp')
    parser.add_argument('--template', default=None, help='default parameters',
                        metavar='template.inp')
    parser.add_argument('--potential', default=0.5, type=float,
                        metavar='float', help='trap potential')
    opts = parser.parse_args(args)
    return opts

opts = get_arguments()

image = QtGui.QImage(opts.image)
image = image.convertToFormat(QtGui.QImage.Format_Mono,
                              QtCore.Qt.ThresholdDither)
image = image.mirrored(0, 1)

w, h = image.width(), image.height()

imap = lm.grid.IndexMapper(w, h, 1)

if opts.template:
    chk = lm.checkpoint.load(opts.template)
else:
    chk = lm.checkpoint.CheckPoint()

for i in range(w):
    for j in range(h):
        c = QtGui.QColor(image.pixel(i, j))
        if not c.value():
            s = imap.indexS(i, j, 0)
            chk.traps.append(s)

chk['trap.potential'] = opts.potential
chk['grid.x'] = w
chk['grid.y'] = h
chk['grid.z'] = 1
chk.fix_traps()

chk.save(opts.inp)