# -*- coding: utf-8 -*-
"""
extend.py
=========

.. argparse::
    :module: extend
    :func: create_parser
    :prog: extend

.. moduleauthor:: Adam Gagorik <adam.gagorik@gmail.com>
"""
import langmuir as lm
from langmuir.common import ccodes
import itertools
import argparse
import sys
import os

desc = """
Extend a set of simulations.
"""

def create_parser():
    parser = argparse.ArgumentParser()
    parser.description = desc
    parser.add_argument('--ok', action='store_true', help='turn off dry run')
    parser.add_argument('-r', action='store_true', help='search recursivly')
    parser.add_argument('--add', default=0, type=int, help='steps to add to iterations.real')
    parser.add_argument('--set', default=None, type=int, help='set iterations.real to new value')
    parser.add_argument('--print', dest='pstep', default=None, type=int, help='set iterations.print to new value')
    parser.add_argument('--stub', default='ext.{i}', help='simulation job name stub')
    parser.add_argument('name', default=None, nargs='?', help='output file name')
    return parser

def get_arguments(args=None):
    parser = create_parser()
    opts = parser.parse_args(args)
    return opts

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    all_parts, last_parts = lm.find.parts(work, r=opts.r), []
    for sim, parts in itertools.groupby(all_parts, key=lambda x : os.path.dirname(x)):
        parts = [part for part in parts]
        parts.sort(key=lm.regex.numbers)
        last_parts.append(parts[-1])
    parts = last_parts

    if not parts:
        chk = lm.find.chk(work)
        try:
            parts = [lm.find.slice_part(work)]
        except AssertionError:
            parts = []

    if not parts:
        print ccodes.red('error: can not find any parts, try using -r')
        sys.exit(-1)

    new_parts, errors = [], 0
    for i, part in enumerate(parts):
        if i > 0:
            print ''

        part_relpath = os.path.relpath(part, work)
        print ccodes.yellow('found: ' +  part_relpath)

        chk = lm.find.chk(part)
        if not chk:
            print ccodes.red('error:') + ' ' + ccodes.blue('can not find chk!')
            errors += 1
            continue
        else:
            chk = lm.checkpoint.load(chk)

        if opts.pstep:
            chk['iterations.print'] = opts.pstep

        if opts.set:
            chk['iterations.real'] = opts.set
        else:
            try:
                chk['iterations.real'] += opts.add
            except KeyError:
                chk['iterations.real'] = opts.add

        try:
            cstep = chk['current.step']
        except KeyError:
            cstep = 0

        try:
            tstep = chk['iterations.real']
        except KeyError:
            tstep = -1

        try:
            pstep = chk['iterations.print']
        except KeyError:
            pstep = 1000

        if not tstep % pstep == 0:
            print ccodes.red('error:') + ' ' + ccodes.blue('iterations.real % iterations.print != 0') + \
                ' ' + '(' + ccodes.cyan('%d' % tstep) + ' % ' + ccodes.cyan('%d' % pstep) + ' = ' + \
                ccodes.cyan('%d' % (tstep % pstep)) + ')'
            errors += 1
            continue

        if tstep <= cstep:
            print ccodes.red('error:') + ' ' + ccodes.blue('iterations.real <= current.step') + \
                ' ' + '(' + ccodes.cyan('%d' % tstep) + ' <= ' + ccodes.cyan('%d' % cstep) + ')'
            errors += 1
            continue

        part_id = lm.regex.part(part_relpath)
        new_part = os.path.join(os.path.dirname(part), 'part.%d' % (part_id + 1))

        if os.path.exists(new_part):
            print ccodes.red('error:') + ' ' + ccodes.blue('part already exists?') + ' ' + \
                ccodes.blue(os.path.relpath(new_part, work))
            errors += 1
            continue

        if opts.ok:
            os.mkdir(new_part)
            chk.save(os.path.join(new_part, 'sim.inp'))

        new_parts.append(new_part)

        print '       ' + ccodes.cyan(os.path.relpath(new_part, work)) + ' ' + \
            '(' + 'current.step=' + ccodes.cyan('%d' % cstep) + ')' + ' ' + \
            '(' + 'iterations.real=' + ccodes.cyan('%d' % tstep) + ')' + ' ' + \
            '(' + 'iterations.print=' + ccodes.cyan('%d' % pstep) + ')'

    print ''
    print 'old sims:', ccodes.c('%d' % len(parts))
    print 'new sims:', ccodes.c('%d' % len(new_parts))
    print 'errors  :', ccodes.c('%d' % errors)

    if not opts.ok:
        print ''
        print ccodes.r('warning: nothing was done, use --ok to turn off dry run')
    else:
        if new_parts:
            lm.common.command_script(new_parts, name=opts.name, stub=opts.stub)