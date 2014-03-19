# -*- coding: utf-8 -*-
"""
script.py
=========

.. argparse::
    :module: script
    :func: create_parser
    :prog: script

.. moduleauthor:: Adam Gagorik <adam.gagorik@gmail.com>
"""
import langmuir as lm
import argparse
import sys
import os

desc = """
Create a bash script for submitting simulations.
"""

def create_parser():
    parser = argparse.ArgumentParser()
    parser.description = desc
    parser.add_argument('--stub', default='{relpath}', help='simulation job name stub')
    parser.add_argument('--mode', default='all', choices=['runs', 'parts', 'all'], help='directory search mode')
    parser.add_argument('--depth', default=None, type=int, help='search depth')
    parser.add_argument('--comment', default=False, action='store_true', help='comment custom command')
    parser.add_argument('name', default=None, nargs='?', help='output file name')
    return parser

def get_arguments(args=None):
    parser = create_parser()
    opts, kwargs = parser.parse_known_args(args)

    subparser = argparse.ArgumentParser()
    for kwarg in kwargs:
        if kwarg.startswith('-'):
            subparser.add_argument(kwarg.split('=')[0])
    subopts = subparser.parse_args(kwargs)

    return opts, subopts.__dict__

if __name__ == '__main__':
    work = os.getcwd()
    opts, kwargs = get_arguments()

    if opts.mode == 'runs':
        paths = lm.find.runs(work, r=True)
    elif opts.mode == 'parts':
        paths = lm.find.parts(work, r=True)
    else:
        paths = lm.find.find(work, r=True, single=False, exclude_files=True)

    if not opts.depth is None:
        paths = [path for path in paths if lm.find.depth(path, work) == opts.depth]

    if not paths:
        print 'no paths found! (mode=%s, depth=%s)' % (opts.mode, opts.depth)
        sys.exit(-1)

    for path in paths:
        print 'depth=%d : %s' % (lm.find.depth(path, work), os.path.relpath(path, work))

    lm.common.command_script(paths, name=opts.name, stub=opts.stub, comment=opts.comment, **kwargs)