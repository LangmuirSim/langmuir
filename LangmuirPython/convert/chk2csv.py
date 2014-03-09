# -*- coding: utf-8 -*-
"""
chk2csv.py
==========

.. argparse::
    :module: chk2csv
    :func: create_parser
    :prog: chk2csv.py

.. moduleauthor:: Adam Gagorik <adam.gagorik@gmail.com>
"""
import langmuir as lm
import pandas as pd
import collections
import argparse
import os

desc = \
"""
Create csv files using data in checkpoint file.
"""

def create_parser():
    parser = argparse.ArgumentParser(description=desc)
    parser.add_argument(dest='input', help='input file name')
    parser.add_argument('--stub', default='', type=str,
                        help='output file name stub')
    parser.add_argument('--positions', action='store_true',
                        help='use true positions')
    return parser

def get_arguments(args=None):
    parser = create_parser()
    opts = parser.parse_args(args)
    return opts

if __name__ == '__main__':

    work = os.getcwd()
    opts = get_arguments()
    chk  = lm.checkpoint.load(opts.input)
    grid = lm.grid.Grid.from_checkpoint(chk)

    if chk.traps:
        print 'found traps'
        print 'mapping indices'
        if chk.potentials:
            xyzv = lm.grid.XYZV(grid, chk.traps, chk.potentials)

        elif chk.parameters.has_key('trap.potential'):
            xyzv = lm.grid.XYZV(grid, chk.traps, chk['trap.potential'])

        else:
            xyzv = lm.grid.XYZV(grid, chk.traps)

        frame = collections.OrderedDict()
        if opts.positions:
            frame.update(x=xyzv.x)
            frame.update(y=xyzv.y)
            frame.update(z=xyzv.z)
            frame.update(v=xyzv.v)
        else:
            frame.update(x=xyzv.xi)
            frame.update(y=xyzv.yi)
            frame.update(z=xyzv.zi)
            frame.update(v=xyzv.v)

        file_name = '%straps.csv' % opts.stub
        frame = pd.DataFrame(frame, columns=frame.keys())
        frame.to_csv(file_name, sep=' ', index=False)
        print 'saved %s\n' % file_name
    else:
        print 'no traps found...\n'

    if chk.defects:
        print 'found defects'
        print 'mapping indices'
        xyzv = lm.grid.XYZV(grid, chk.defects, 0)

        frame = collections.OrderedDict()
        if opts.positions:
            frame.update(x=xyzv.x)
            frame.update(y=xyzv.y)
            frame.update(z=xyzv.z)
            frame.update(v=xyzv.v)
        else:
            frame.update(x=xyzv.xi)
            frame.update(y=xyzv.yi)
            frame.update(z=xyzv.zi)
            frame.update(v=xyzv.v)

        file_name = '%sdefcs.csv' % opts.stub
        frame = pd.DataFrame(frame, columns=frame.keys())
        frame.to_csv(file_name, sep=' ', index=False)
        print 'saved %s\n' % file_name
    else:
        print 'no defects found...\n'

    if chk.electrons:
        print 'found electrons'
        print 'mapping indices'
        xyzv = lm.grid.XYZV(grid, chk.electrons, -1)

        frame = collections.OrderedDict()
        if opts.positions:
            frame.update(x=xyzv.x)
            frame.update(y=xyzv.y)
            frame.update(z=xyzv.z)
            frame.update(v=xyzv.v)
        else:
            frame.update(x=xyzv.xi)
            frame.update(y=xyzv.yi)
            frame.update(z=xyzv.zi)
            frame.update(v=xyzv.v)

        file_name = '%selecs.csv' % opts.stub
        frame = pd.DataFrame(frame, columns=frame.keys())
        frame.to_csv(file_name, sep=' ', index=False)
        print 'saved %s\n' % file_name
    else:
        print 'no electrons found...\n'

    if chk.holes:
        print 'found holes'
        print 'mapping indices'
        xyzv = lm.grid.XYZV(grid, chk.holes, 1)

        frame = collections.OrderedDict()
        if opts.positions:
            frame.update(x=xyzv.x)
            frame.update(y=xyzv.y)
            frame.update(z=xyzv.z)
            frame.update(v=xyzv.v)
        else:
            frame.update(x=xyzv.xi)
            frame.update(y=xyzv.yi)
            frame.update(z=xyzv.zi)
            frame.update(v=xyzv.v)

        file_name = '%sholes.csv' % opts.stub
        frame = pd.DataFrame(frame, columns=frame.keys())
        frame.to_csv(file_name, sep=' ', index=False)
        print 'saved %s' % file_name
    else:
        print 'no holes found...'