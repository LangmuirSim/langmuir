# -*- coding: utf-8 -*-
"""
done.py
=======

.. argparse::
    :module: done
    :func: create_parser
    :prog: done

.. moduleauthor:: Adam Gagorik <adam.gagorik@gmail.com>
"""
import langmuir as lm
import argparse
import os

desc = """
Search for dat files and report on their progress.
"""

def create_parser():
    parser = argparse.ArgumentParser()
    parser.description = desc
    parser.add_argument('-r', action='store_true', help='search recursivly')
    return parser

def get_arguments(args=None):
    parser = create_parser()
    opts = parser.parse_args(args)
    return opts

def search(work, r=False):
    dats = lm.find.dats(work, r=r)
    chks = lm.find.chks(work, r=r)
    dirs = [os.path.abspath(os.path.dirname(d)) for d in dats]
    for chk in chks:
        d = os.path.dirname(chk)
        if not d in dirs:
            dirs.append(d)
    dirs.sort(key=lm.regex.numbers)

    chks = []
    dats = []
    for d in dirs:
        try:
            chk = os.path.relpath(lm.find.chk(d), work)
        except (RuntimeError, TypeError):
            chk = None

        try:
            dat = os.path.relpath(lm.find.dat(d), work)
        except (RuntimeError, TypeError):
            dat = None

        chks.append(chk)
        dats.append(dat)

    return chks, dats

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    chks, dats = search(work, r=opts.r)

    if chks or dats:
        print '{:>45}{:>10}{:>10}{:>10}'.format('path', 'current', 'total', 'percent')
        print '-' * 80
        for i, (chk, dat) in enumerate(zip(chks, dats)):
            try:
                path = os.path.dirname(chk)
            except AttributeError:
                try:
                    path = os.path.dirname(dat)
                except AttributeError:
                    path = os.path.join(work, '???')
            path = os.path.relpath(path, work)

            chk = lm.checkpoint.load(chk)
            try:
                cstep = chk['current.step']
            except KeyError:
                cstep = None

            try:
                tstep = chk['iterations.real']
            except KeyError:
                tstep = None

            try:
                percent = float(cstep)/float(tstep) * 100
            except TypeError:
                percent = 0.00

            print '{path:45}{cstep:10}{tstep:10}{percent:10.5f}'.format(**locals())

            if i == 128:
                break
    else:
        print 'can not find any simulations (use -r)'