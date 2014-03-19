# -*- coding: utf-8 -*-
"""
pepper.py
=========

.. argparse::
    :module: pepper
    :func: create_parser
    :prog: pepper

.. moduleauthor:: Adam Gagorik <adam.gagorik@gmail.com>
"""
import langmuir as lm
import collections
import argparse
import sys
import os

desc = """
Add pepper to phase(s).
"""

def create_parser():
    parser = argparse.ArgumentParser()
    parser.description = desc

    parser.add_argument(dest='ifile', type=str, metavar='input',
        help='input file')
    parser.add_argument(dest='ofile', type=str, metavar='output', default=None, nargs='?',
        help='output file')

    parser.add_argument('--percent', type=float, metavar='float', default=0.0,
        help='pepper conecntration')
    parser.add_argument('--size', type=int, metavar='int', default=1,
        help='pixel size')
    parser.add_argument('--phase', type=str, metavar='str', default='both', choices=['both', '0', '1'],
        help='which phase to add pepper to')

    return parser

def get_arguments(args=None):
    parser = create_parser()
    opts = parser.parse_args(args)
    return opts

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    print opts