# -*- coding: utf-8 -*-
import itertools
import langmuir
import argparse
import os

desc = \
"""
set parameters for simulations
"""

def get_arguments(args=None):
    parser = argparse.ArgumentParser(description=desc)

    parser.add_argument('--recursive', '-r', default=False, action='store_true',
               help='find input files recursively')

    parser.add_argument(dest='stub', default='*', nargs='?',
               help='file matching pattern')

    parser.add_argument('--key', '-k', dest='keys', action='append', default=[],
               nargs='*', metavar='key', help='list of keys')

    parser.add_argument('--value', '--val', '-v', dest='vals', action='append',
               default=[], nargs='*', metavar='val', help='list of values')

    parser.add_argument('--mode', choices=['inps', 'chks'], default='inps',
               help='search mode')

    opts = parser.parse_args(args)

    opts.keys = list(itertools.chain(*opts.keys))
    opts.vals = list(itertools.chain(*opts.vals))

    if not len(opts.keys) == len(opts.vals):
        raise RuntimeError('len(keys) != len(values)')

    for i, (key, value) in enumerate(zip(opts.keys, opts.vals)):
        if not key in langmuir.parameters._valid_keys:
            raise RuntimeError('invalid key : %s' % key)
        col = langmuir.database.parameters[key]
        try:
            if col.pytype is bool:
                if opts.vals[i].lower().startswith('f'):
                    opts.vals[i] = False
                elif opts.vals[i].lower().startswith('t'):
                    opts.vals[i] = True
            else:
                opts.vals[i] = col.pytype(value)
        except ValueError:
            raise RuntimeError("can not convert '%s' to %s" % (value,
                col.pytype.__name__))

    return opts

if __name__ == '__main__':

    opts = get_arguments()
    work = os.getcwd()

    if opts.mode == 'inps':
        sims = langmuir.find.inps(work, opts.stub, recursive=opts.recursive)
    elif opts.mode == 'chks':
        sims = langmuir.find.chks(work, opts.stub, recursive=opts.recursive)
    else:
        raise RuntimeError('invalid search mode...')

    if not sims:
        raise RuntimeError('no sims found...')

    for i, sim in enumerate(sims):
        print i, sim
        chk = langmuir.checkpoint.load(sim)
        for key, value in zip(opts.keys, opts.vals):
            chk[key] = value
        chk.save(sim)

