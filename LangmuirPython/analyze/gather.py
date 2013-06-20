# -*- coding: utf-8 -*-
import pandas as pd
import itertools
import langmuir
import argparse
import os

parser = argparse.ArgumentParser()
parser.description = 'gather output of run from combined.pkl.gz'
parser.add_argument('--stub', default='gathered', help='output file stub')
parser.add_argument('--last', default=-1, type=int, help='last step')
parser.add_argument('--equil', default=None, type=int, help='equil step')
parser.add_argument('-r', dest='recursive', default=False, action='store_true',
           help='gather output recursivly')
opts = parser.parse_args()

def gather(path):
    work = os.getcwd()
    os.chdir(path)
    pkls = langmuir.find.pkls(os.getcwd(), stub='combined*', recursive=True,
                              at_least_one=True)
    parms, gathered = [], []
    print os.path.relpath(path, work)
    for i, pkl in enumerate(pkls):
        sim = os.path.dirname(pkl)
        print '\t(%3d/%3d) : %s' % (i + 1, len(pkls),
            os.path.relpath(sim, path))
        parm = langmuir.find.parms(sim, recursive=True)[-1]
        parm = langmuir.parameters.load(parm)
        parms.append(dict(parm))
        parms[-1]['path'] = sim
        pkl = langmuir.analyze.load_pkl(pkl)
        pkl = langmuir.analyze.equilibrate(pkl, last=opts.last,
                                           equil=opts.equil)
        pkl.name = i
        gathered.append(pkl)

    parms = pd.DataFrame(parms)
    gathered = pd.DataFrame(gathered)

    gathered = gathered.combine_first(parms)
    gathered = langmuir.analyze.calculate(gathered)

    langmuir.analyze.save_pkl(gathered, '%s.pkl.gz' % opts.stub)
    print ''
    os.chdir(work)

if opts.recursive:
    work = os.getcwd()
    combined = langmuir.find.pkls(work, stub='combined*', recursive=True)
    combined = itertools.groupby(combined,
        lambda x : os.path.dirname(os.path.dirname(x)))
    combined = [c for c, b in combined]
    for c in combined:
        gather(c)
else:
    gather(os.getcwd())
