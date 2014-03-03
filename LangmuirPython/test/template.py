# -*- coding: utf-8 -*-
"""
@author: adam
"""
import langmuir as lm
import argparse
import os

desc = """
Program description.
"""

def get_arguments(args=None):
    parser = argparse.ArgumentParser()
    parser.description = desc
    parser.add_argument(dest='ifile', default=None, type=str, nargs='?',
                        metavar='input', help='input file')
    parser.add_argument('--stub', default=None, type=str,
                        metavar='stub', help='output file stub')
    opts = parser.parse_args(args)
    return opts

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()
