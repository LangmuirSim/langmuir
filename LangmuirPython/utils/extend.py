# -*- coding: utf-8 -*-
"""
Created on Wed Apr  3 10:46:29 2013

@author: adam
"""

import itertools
import langmuir
import argparse
import sys
import os

desc = \
"""
Extend a simulation or group of simulations.
"""

def get_arguments(args=None):
    parser = argparse.ArgumentParser(description=desc)

    parser.add_argument(dest='root', default=os.getcwd(), nargs='?',
               help='root directory of simulation(s)')

    parser.add_argument('--recursive', '-r', default=False, action='store_true',
               help='extend simulations in all sub-directories')

    parser.add_argument('--real', default=0, type=int, help='iterations.real')

    parser.add_argument('--print', dest='iprint', default=None, type=int,
               help='iterations.print')

    parser.add_argument('--seed', default=False, action='store_true',
               help='do not set seed.charges = 0.00')

    parser.add_argument('--mode', default=None, type=str, choices=['set', 'add'],
               help='how to handle iterations.real', required=True,
               metavar='set|add')

    parser.add_argument('--key', '-k', dest='keys', default=[], action='append',
               type=str, help='parameter key', metavar='key')

    parser.add_argument('--value', '-v', dest='values', default=[], action='append',
               type=str, help='parameter value', metavar='val')

    parser.add_argument('--stub', dest='stub', type=str, default='run',
               help='stub to use in command script')

    opts = parser.parse_args(args)

    if opts.real == 0:
        print >> sys.stderr, 'error: iterations.real = %d' % opts.real
        print >> sys.stderr, 'error: exiting...'
        sys.exit(-1)

    if opts.iprint:
        if not opts.real % opts.iprint == 0:
            print >> sys.stderr, 'error: %d %% %d != 0' % (opts.real,
                                                           opts.iprint)
            print >> sys.stderr, 'error: invalid iterations.print'
            print >> sys.stderr, 'error: exiting...'
            sys.exit(-1)

    if not len(opts.keys) == len(opts.values):
        print >> sys.stderr, 'error: len(keys) != len(values)'
        print >> sys.stderr, 'error: exiting...'
        sys.exit(-01)
    else:
        for i, (key, value) in enumerate(zip(opts.keys, opts.values)):
            if key in langmuir.parameters._valid_keys:
                try:
                    opts.values[i] = \
                        langmuir.database.parameters[key].pytype(value)
                except ValueError:
                    print >> sys.stderr, 'error: invalid type for value'
                    print >> sys.stderr, 'error: %s' % (key)
                    print >> sys.stderr, 'error: %s' % (value)
                    print >> sys.stderr, 'error: %s' % (
                        langmuir.database.parameters[key].pytype.__name__)
                    print >> sys.stderr, 'error: exiting...'
                    sys.exit(-1)
            else:
                print >> sys.stderr, 'error: invalid key %s' % key
                print >> sys.stderr, 'error: exiting...'
                sys.exit(-1)

    return opts

if __name__ == '__main__':

    opts = get_arguments()
    work = os.getcwd()

    os.chdir(opts.root)

    parts = langmuir.find.parts(work, opts.recursive, at_least_one=True)
    parts = [list(group)[-1] for key, group in itertools.groupby(
        parts, lambda x : os.path.dirname(x))]
    if not parts:
        raise RuntimeError('can not find any simulations')

    chks = [langmuir.find.chk(part, at_least_one=True) for part in parts]

    new_sims = []
    for i, (part, chk) in enumerate(zip(parts, chks)):
        sim = os.path.dirname(part)
        pid = langmuir.regex.part(part)
        new_part = os.path.join(sim, 'part.%d' % (pid + 1))

        chk = langmuir.checkpoint.load(chk)

        if opts.mode == 'set':
            chk['iterations.real'] = opts.real
            print '%s --> %s iterations.real = %d' % (
                os.path.relpath(part, opts.root),
                os.path.relpath(new_part, opts.root), chk['iterations.real'])
        elif opts.mode == 'add':
            chk['iterations.real'] += opts.real
            print '%s --> %s iterations.real = %d (+=%d)' % (
                os.path.relpath(part, opts.root),
                os.path.relpath(new_part, opts.root), chk['iterations.real'],
                opts.real)
        else:
            raise RuntimeError('unknown mode : %s' % opts.mode)

        if opts.seed:
            pass
        else:
            chk['seed.charges'] = 0.00

        if opts.iprint:
            chk['iterations.print'] = opts.iprint

        for key, value in zip(opts.keys, opts.values):
            chk[key] = value

        if chk['current.step'] >= chk['iterations.real']:
            print >> sys.stderr, 'error: current.step = %d' % \
                chk['current.step']
            print >> sys.stderr, 'error: iterations.real = %d' % \
                chk['iterations.real']
            print >> sys.stderr, 'error: skipping simulation'
        else:
            os.mkdir(new_part)
            os.chdir(new_part)
            chk.save('sim.inp')
            new_sims.append(os.path.join(new_part))
            os.chdir(opts.root)

    if not new_sims:
        raise RuntimeError('no simulations have been extended')
    else:
        langmuir.common.command_script(new_sims, name='submit',
                                       stub=opts.stub)
        print 'saved submit script'

    os.chdir(work)
