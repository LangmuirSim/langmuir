# -*- coding: utf-8 -*-
"""
equil.py
========

.. argparse::
    :module: time
    :func: create_parser
    :prog: time

.. moduleauthor:: Adam Gagorik <adam.gagorik@gmail.com>
"""
import langmuir as lm
import pandas as pd
import collections
import argparse
import os

desc = """
Check simulation(s) for time data.
"""

def create_parser():
    parser = argparse.ArgumentParser()
    parser.description = desc

    parser.add_argument(dest='pkls', default=[], type=str, nargs='*',
        metavar='file', help='input files')

    parser.add_argument('--mode', default='pkl', choices=['dat', 'pkl'])

    parser.add_argument('-r', action='store_true',
        help='search for files recursivly')

    parser.add_argument('--last', default=None, type=int, metavar='#',
                        help='last step')

    parser.add_argument('--equil', default=None, type=int, metavar='#',
                        help='equil step')

    return parser

def get_arguments(args=None):
    parser = create_parser()
    opts = parser.parse_args(args)

    if not opts.pkls:
        opts.pkls = [os.getcwd()]

    pkls = []
    for pkl in opts.pkls:
        if os.path.isdir(pkl):
            if opts.mode == 'pkl':
                pkls.extend(lm.find.pkls(pkl, stub='combined*', r=opts.r))
            if opts.mode == 'dat':
                pkls.extend(lm.find.dats(pkl, stub='out*', r=opts.r))
        else:
            pkls.append(pkl)
    opts.pkls = pkls

    if not opts.pkls:
        raise RuntimeError('can not find any files')

    return opts

columns = [
    'simulation:time',
    'simulation:time:i',
    'simulation:time:f',
    'real:time',
    'real:time:i',
    'real:time:f',
    'grid.x',
    'grid.y',
    'grid.z',
    'coulomb.carriers',
    'coulomb.gaussian.sigma',
    'use.opencl',
    'work.size',
    'max.threads'
]

for stub in ['electron', 'hole', 'carrier']:
    columns.append('%s:avg' % stub)
    columns.append('%s:std' % stub)
    columns.append('%s:min' % stub)
    columns.append('%s:max' % stub)
    columns.append('%s:rng' % stub)
    columns.append('%s:i'   % stub)
    columns.append('%s:f'   % stub)

def try_set(pkl, pkey, table, tkey=None, convert=lambda x: x):
    if tkey is None:
        tkey = pkey

    if tkey in table:
        try:
            table[tkey].append(convert(pkl[pkey]))
        except:
            table[tkey].append(None)
    else:
        raise RuntimeError('invalid table column: %s' % tkey)

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    table = collections.OrderedDict()
    for column in columns:
        table[column] = []

    for i, path in enumerate(opts.pkls):
        print '[%4d of %4d] [%6.2f %%]' % (i + 1, len(opts.pkls), float(i + 1)/len(opts.pkls)*100.0)

        if opts.mode == 'dat':
            pkl = lm.datfile.load(path)

        if opts.mode == 'pkl':
            pkl = lm.common.load_pkl(path)

        pkl = lm.analyze.calculate(pkl)
        sub = pkl.iloc[opts.equil:opts.last]
        eql = sub.iloc[-1] - sub.iloc[0]

        avg = sub.mean()
        std = sub.std()
        min = sub.min()
        max = sub.max()
        rng = abs(max - min)

        try_set(eql, 'simulation:time', table, convert=int)
        try_set(sub.iloc[ 0], 'simulation:time', table, 'simulation:time:i', convert=int)
        try_set(sub.iloc[-1], 'simulation:time', table, 'simulation:time:f', convert=int)

        try_set(eql, 'real:time', table, convert=int)
        try_set(sub.iloc[ 0], 'real:time', table, 'real:time:i', convert=int)
        try_set(sub.iloc[-1], 'real:time', table, 'real:time:f', convert=int)

        for stub in ['electron', 'hole', 'carrier']:
            pkey = '%s:count' % stub
            try_set(avg, pkey, table, '%s:avg' % stub, convert=float)
            try_set(std, pkey, table, '%s:std' % stub, convert=float)
            try_set(min, pkey, table, '%s:min' % stub, convert=int)
            try_set(max, pkey, table, '%s:max' % stub, convert=int)
            try_set(rng, pkey, table, '%s:rng' % stub, convert=int)
            try_set(sub.iloc[0 ], pkey, table, '%s:i' % stub, convert=int)
            try_set(sub.iloc[-1], pkey, table, '%s:f' % stub, convert=int)

        dirname = os.path.dirname(path)
        chk = lm.find.parms(dirname, r=True)
        if not chk:
            chk = lm.find.chks(dirname, r=True)
            if not chk:
                chk = [None]
        chk = chk[-1]

        try:
            chk = lm.checkpoint.load(chk)
        except TypeError:
            chk = lm.checkpoint.CheckPoint()

        try_set(chk, 'grid.x', table, convert=int)
        try_set(chk, 'grid.y', table, convert=int)
        try_set(chk, 'grid.z', table, convert=int)
        try_set(chk, 'coulomb.gaussian.sigma', table, convert=float)
        try_set(chk, 'coulomb.carriers', table, convert=bool)
        try_set(chk, 'max.threads', table, convert=int)
        try_set(chk, 'use.opencl', table, convert=bool)
        try_set(chk, 'work.size', table, convert=bool)

    table = pd.DataFrame(table, columns=table.keys())
    table.to_csv('time.csv', index=False)