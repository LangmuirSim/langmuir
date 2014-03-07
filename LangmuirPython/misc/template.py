# -*- coding: utf-8 -*-
"""
template.py
===========

.. argparse::
    :module: template
    :func: create_parser
    :prog: template

.. moduleauthor:: Adam Gagorik <adam.gagorik@gmail.com>
"""
import argparse
import os

desc = """
Program description.
"""

def create_parser():
    parser = argparse.ArgumentParser()
    parser.description = desc
    parser.add_argument(dest='ifile', default=None, type=str, nargs='?',
                        metavar='input', help='input file')
    parser.add_argument('--stub', default=None, type=str,
                        metavar='stub', help='output file stub')
    return parser

def get_arguments(args=None):
    parser = create_parser()
    opts = parser.parse_args(args)
    return opts

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()
