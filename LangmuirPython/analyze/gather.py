# -*- coding: utf-8 -*-
"""
gather.py
=========

.. argparse::
    :module: gather
    :func: create_parser
    :prog: gather.py

.. moduleauthor:: Adam Gagorik <adam.gagorik@gmail.com>
"""
import langmuir as lm
import pandas as pd
import itertools
import argparse
import os

desc = """
Gather the output of a series of pickles.  The pickes should have been made
using 'combine.py'.
""".lstrip()

def create_parser():
    parser = argparse.ArgumentParser()
    parser.description = desc
    parser.add_argument('--stub', default='gathered', type=str,
                        metavar='stub', help='output file stub')
    parser.add_argument('--last', default=-1, type=int, metavar='#',
                        help='last step')
    parser.add_argument('--equil', default=None, type=int, metavar='#',
                        help='equil step')
    parser.add_argument('-g', action='store_true',
       help='gathered output is grouped by os.dirname')
    return parser

def get_arguments(args=None):
    parser = create_parser()
    opts = parser.parse_args(args)
    return opts

def gather(path):
    work = os.getcwd()
    os.chdir(path)
    pkls = lm.find.pkls(os.getcwd(), r=True, stub='combined*',
        at_least_one=True)
    parms, gathered = [], []
    print os.path.abspath(path)
    for i, pkl in enumerate(pkls):
        sim = os.path.dirname(pkl)
        print '\t(%3d/%3d) : %s' % (i + 1, len(pkls),
            os.path.relpath(sim, path))
        parm = lm.find.parms(sim, r=True)[-1]
        parm = lm.parameters.load(parm)
        parms.append(dict(parm))
        parms[-1]['path'] = sim
        pkl = lm.common.load_pkl(pkl)
        pkl = lm.analyze.equilibrate(pkl, last=opts.last, equil=opts.equil)
        pkl.name = i
        gathered.append(pkl)

    parms = pd.DataFrame(parms)
    gathered = pd.DataFrame(gathered)

    gathered = gathered.combine_first(parms)
    gathered = lm.analyze.calculate(gathered)

    lm.common.save_pkl(gathered, '%s.pkl.gz' % opts.stub)

    os.chdir(work)

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    if opts.g:
        combined = lm.find.pkls(work, stub='combined*', r=True,
            at_least_one=True)
        combined.sort(key=lambda x : os.path.dirname(os.path.dirname(x)))          
        combined = itertools.groupby(combined,
            lambda x : os.path.dirname(os.path.dirname(x)))
        combined = [c for c, b in combined]       
        for c in combined:
            gather(c)
    else:
        gather(work)