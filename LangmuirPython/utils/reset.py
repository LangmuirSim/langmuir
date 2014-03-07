# -*- coding: utf-8 -*-
"""
reset.py
========

.. argparse::
    :module: reset
    :func: create_parser
    :prog: reset.py

.. moduleauthor:: Adam Gagorik <adam.gagorik@gmail.com>
"""
import langmuir as lm
import argparse
import os

desc = """
Reset a checkpoint file to be more like a new simulation.
"""

def create_parser():
    parser = argparse.ArgumentParser()
    parser.description = desc
    parser.add_argument(dest='ifile', default='sim.inp', type=str, nargs='?',
                        metavar='input', help='input file')
    parser.add_argument(dest='ofile', default=None, type=str, nargs='?',
                        metavar='output', help='output file')
    parser.add_argument('--elecs', action='store_true', default=False,
                        help='keep electrons')
    parser.add_argument('--holes', action='store_true', default=False,
                        help='keep holes')
    parser.add_argument('--notraps', action='store_true', default=False,
                        help='delete traps')
    parser.add_argument('--nodefects', action='store_true', default=False,
                        help='delete defects')
    parser.add_argument('--nofix', action='store_true', default=False,
                        help='do not fix traps')
    return parser

def get_arguments(args=None):
    parser = create_parser()
    opts = parser.parse_args(args)
    if opts.ofile is None:
        opts.ofile = opts.ifile
    return opts

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    chk  = lm.checkpoint.load(opts.ifile)
    chk.reset(keep_elecs=opts.elecs, keep_holes=opts.holes)

    if opts.notraps:
        chk.traps = []
        chk.potentials = []
        if not opts.nofix:
            chk['trap.percentage'] = 0.0
            chk['trap.potential' ] = 0.0

    if opts.nodefects:
        chk.defects = []
        chk['defect.percentage'] = 0.0

    if not opts.nofix:
        chk.fix_traps()

    chk.save(opts.ofile)

    print chk
    if opts.ifile is opts.ofile:
        print 'reset', opts.ofile
    else:
        print 'saved', opts.ofile