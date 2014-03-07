# -*- coding: utf-8 -*-
"""
create_rst.py
=============

.. argparse::
    :module: create_rst
    :func: create_parser
    :prog: create_rst.py

.. moduleauthor:: Adam Gagorik <adam.gagorik@gmail.com>
"""
import argparse
import os

desc = """
Create rst files from all .py files found in path.
"""

def create_parser():
    parser = argparse.ArgumentParser()
    parser.description = desc
    parser.add_argument(dest='ipath', nargs='?', default=None,
        help='path to search')
    parser.add_argument(dest='opath', nargs='?', default=None,
        help='path to write' )
    parser.add_argument('-r', action='store_true', help='search recursivly')
    return parser

def get_arguments(args=None):
    parser = create_parser()
    opts = parser.parse_args(args)
    if opts.ipath is None: opts.ipath = os.getcwd()
    if opts.opath is None: opts.opath = os.getcwd()
    return opts

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    sdir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))

    found = []

    for root, dirs, files in os.walk(opts.ipath):
        for f in files:
            stub, ext = os.path.splitext(f)
            if ext == '.py':
                found.append(os.path.relpath(os.path.join(root, f), sdir))
        if not opts.r:
            break

    for f in found:
        fdir = os.path.join(opts.opath, os.path.dirname(f))

        if not os.path.exists(fdir):
            os.makedirs(fdir)

        stub, ext = os.path.splitext(f)
        with open(stub + '.rst', 'w') as handle:
            print >> handle, r'.. automodule:: %s' % os.path.basename(stub)
            print >> handle, r'    :members:'

        print os.path.join('scripts', stub)