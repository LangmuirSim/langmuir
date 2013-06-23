# -*- coding: utf-8 -*-
import itertools
import argparse
import langmuir
import os

parser = argparse.ArgumentParser()
parser.description = 'combine output of parts'
parser.add_argument(dest='root', default=os.getcwd(), nargs='?',
                    help='root directory')
parser.add_argument('-r', dest='recursive', default=False, action='store_true',
                    help='combine parts recursivly')
opts = parser.parse_args()

work = os.getcwd()
parts = langmuir.find.parts(opts.root, recursive=opts.recursive,
                            at_least_one=True)
groups = []
for sim, parts in itertools.groupby(parts, lambda x : os.path.dirname(x)):
    dats = []
    for part in parts:
        dat = langmuir.find.dat(part, at_least_one=True)
        dats.append(dat)
    groups.append((sim, parts, dats))

for i, (sim, parts, dats) in enumerate(groups):
    print '(%3d/%3d) : %s' % (i + 1, len(groups),
        os.path.relpath(sim, opts.root))
    os.chdir(sim)
    combined = langmuir.analyze.combine(dats)
    langmuir.analyze.save_pkl(combined, 'combined.pkl.gz')
    os.chdir(work)
