# -*- coding: utf-8 -*-
"""
combine.py
==========

.. argparse::
    :module: combine
    :func: create_parser
    :prog: combine.py

.. code-block:: bash

    adam@work:~$ ls
    part.0 part.1
    adam@work:~$ python combine -r
    (  1/  1) : .
    adam@work:~$ ls
    combined.pkl.gz part.0 part.1

.. moduleauthor:: Adam Gagorik <adam.gagorik@gmail.com>
"""
import langmuir as lm
import itertools
import argparse
import os

desc = r"""
Combine the output of a series of dat files.  The dat files should be in a
series of directories labeled part.0, part.1, part.2, etc.
"""

def create_parser():
    parser = argparse.ArgumentParser()
    parser.description = desc
    parser.add_argument('-r', action='store_true',
       help='search recursivly for parts')
    return parser

def get_arguments(args=None):
    parser = create_parser()
    opts = parser.parse_args(args)
    return opts

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    parts  = lm.find.parts(work, r=opts.r)
    if not parts:
        print 'no parts found...'

    groups = []
    for sim, parts in itertools.groupby(parts, lambda x : os.path.dirname(x)):
        dats = []
        for part in parts:
            dat = lm.find.dat(part, at_least_one=True)
            dats.append(dat)
        groups.append((sim, parts, dats))

    for i, (sim, parts, dats) in enumerate(groups):
        print '(%3d/%3d) : %s' % (i + 1, len(groups),
            os.path.relpath(sim, work))
        os.chdir(sim)
        combined = lm.analyze.combine(dats)
        lm.common.save_pkl(combined, 'combined.pkl.gz')
        os.chdir(work)
