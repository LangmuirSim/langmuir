# -*- coding: utf-8 -*-
"""
@author: adam
"""
import langmuir as lm
import itertools
import argparse
import os

desc = """
Combine the output of a series of dat files.  The dat files should be in a
series of directories labeled part.0, part.1, part.2, etc.
""".lstrip()

def get_arguments(args=None):
    parser = argparse.ArgumentParser()
    parser.description = desc
    parser.add_argument('-r', action='store_true',
       help='search recursivly for parts')
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