# -*- coding: utf-8 -*-
"""
scan.py
=======

.. argparse::
    :module: scan
    :func: create_parser
    :prog: scan.py

.. moduleauthor:: Adam Gagorik <adam.gagorik@gmail.com>
"""
from subprocess import check_call
import langmuir as lm
import numpy as np
import argparse
import glob
import sys
import os
import re

desc = \
"""
scan a working variable - simply set a parameter in the input file equal to
a python list or numpy array.  For example, voltage.right = [-1, -2, -3]
"""

def create_parser():
    parser = argparse.ArgumentParser(description=desc)

    parser.add_argument(dest='template', default='sim.inp', nargs='?',
        help='template input file name')

    parser.add_argument('--real', dest='real', type=int, metavar='int',
        default=None, help='iterations.real when --fresh or --equil are not set')

    parser.add_argument('--fresh', dest='fresh', type=int, metavar='int',
        default=None, help='override iterations.real for fresh runs')

    parser.add_argument('--equil', dest='equil', type=int, metavar='int',
        default=None, help='override iterations.real for equilibrated runs')

    parser.add_argument('--print', dest='iprint', type=int, metavar='int',
        default=1000, help='iterations.print')

    parser.add_argument('--sort', default=False, action='store_true',
        help='sort the working variable values')

    parser.add_argument('--key', default=None, type=str, choices=['None', 'abs'],
        help='key for sorting working variable values (None or abs)')

    parser.add_argument('--reverse', default=False, action='store_true',
        help='reverse the working variable values')

    parser.add_argument('--mode', dest='mode', type=str, metavar='str',
        default=None, choices=['gen', 'scan'], help='what to do (gen or scan)')

    parser.add_argument('--job', dest='job', default='run', type=str,
        help='stub to use in command script')

    parser.add_argument('--stub', dest='stub', type=str,
        default='{working}_{value}', help='stub to use for directories; '
        + 'you can use {working}, {value}, {i} in the string')

    parser.add_argument('--fmt', default=None, type=str,
        help='value format string in stub')

    return parser

def get_arguments(args=None):
    parser = create_parser()
    opts = parser.parse_args(args)

    if opts.key == 'None':
        opts.key = None

    if opts.equil is None:
        opts.equil = opts.real

    if opts.fresh is None:
        opts.fresh = opts.real

    if opts.equil is None:
        raise RuntimeError('invalid iterations.real: set --real or --equil')

    if opts.fresh is None:
        raise RuntimeError('invalid iterations.real: set --real or --fresh')

    if not os.path.exists(opts.template):
        raise RuntimeError('%s does not exist' % opts.template)

    if not os.path.samefile(os.path.abspath(os.path.dirname(
        opts.template)), os.getcwd()):
        raise RuntimeError('run scan.py in same directory as the input file')

    if not opts.equil % opts.iprint == 0:
        raise RuntimeError('invalid iterations.print: %d %% %d != 0' % (
            opts.equil, opts.iprint))

    if not opts.fresh % opts.iprint == 0:
        raise RuntimeError('invalid iterations.print: %d %% %d != 0' % (
            opts.fresh, opts.iprint))

    if not any(ss in opts.stub for ss in ['{working}', '{value}', '{i}']):
        raise RuntimeError("invalid directory stub: must use one of {working},"
            + " {value}, or {i} in --stub;\n\t"
            + "for example --stub '{working}_{value}' or --stub 'sim.{i}'")

    return opts

class Scanner:
    def __init__(self, opts):
        """
        Class to scan a working variable
        """
        self.checkpoint = None
        self.working    = None
        self.values     = None
        self.load(opts)

    def load(self, opts):
        """
        Parse langmuir input file and look for a working variable
        """
        self.checkpoint = lm.checkpoint.load(opts.template)

        for key, value in self.checkpoint.parameters.iteritems():
            if isinstance(value, (list, tuple, np.ndarray)):
                if self.working is None:
                    self.working = key
                    self.values  = list(value)
                else:
                    raise RuntimeError(
                            'multiple working variables not supported.\n'
                            'found : %s and %s' % (self.working, key))
        if self.working is None:
            raise RuntimeError(
                    'no working variable found in input file. Supported '
                    'working varibale types are python lists, tuples, or'
                    ' numpy arrays.')

        if opts.sort:
            print 'sorted working values!'
            if opts.key is None:
                self.values.sort()
            elif opts.key == 'abs':
                self.values.sort(key=lambda x : abs(x))
            else:
                raise RuntimeError('unknown sort method : %s' % opts.sortkey)

        if opts.reverse:
            print 'reversed working values!'
            self.values.reverse()

        if not opts.fmt is None:
            re.sub('{value}', '{value:{fmt}}', opts.stub)

        self.checkpoint[self.working] = self.values[0]

    def generate(self, opts):

        values = list(set(self.values))
        values.sort(key = lambda x : self.values.index(x))

        sims = []
        for i, value in enumerate(values):
            path = opts.stub.format(working=self.working, value=value, i=i,
                fmt=opts.fmt)
            sims.append(os.path.join(work, path))

        paths = []
        for i, (value, sim) in enumerate(zip(values, sims)):

            path = os.path.join(sim, 'part.0')
            paths.append(path)

            if os.path.exists(path):
                print '%s [%3d/%3d] %s' % (
                    os.path.relpath(sim, work), i + 1, len(values),
                        'nothing to be done')
            else:

                self.checkpoint.reset(keep_elecs=False, keep_holes=False)
                self.checkpoint['iterations.real' ] = opts.fresh
                self.checkpoint['iterations.print'] = opts.iprint
                self.checkpoint[self.working      ] = value

                os.makedirs(path)
                self.checkpoint.save(os.path.join(path, 'sim.inp'))

                print '%s [%3d/%3d] %s' % (
                    os.path.relpath(sim, work), i + 1, len(values),
                        'fresh simulation')

        lm.common.command_script(paths, name='submit', stub=opts.job)

    def scan(self, opts):

        sims = []
        for i, value in enumerate(self.values):
            path = opts.stub.format(working=self.working, value=value, i=i,
                fmt=opts.fmt)
            sims.append(os.path.join(work, path))

        for this_index, (this_value, this_sim) in enumerate(zip(
            self.values, sims)):

            last_index = this_index - 1
            this_path = None
            last_sim = None
            this_chk = None

            if last_index >= 0:
                last_sim = sims[last_index]

            if os.path.exists(this_sim):
                last_chk = lm.checkpoint.load_last(this_sim)
                if last_chk['current.step'] < opts.equil:
                    this_chk = last_chk
                    this_chk['seed.charges'     ] = 0.0
                    this_chk['iterations.real'  ] = opts.equil
                    this_chk['iterations.print' ] = opts.iprint
                    this_chk[self.working       ] = this_value
                    last_part = lm.find.parts(this_sim,
                                                    at_least_one=True)[-1]
                    this_path = os.path.join(this_sim, 'part.%d' % (
                        lm.regex.part(last_part) + 1))
                    os.makedirs(this_path)
                    this_path = os.path.join(this_path, 'sim.inp')
                    this_chk.save(this_path)

                    print '%s [%3d/%3d] %s' % (
                        os.path.relpath(this_sim, work), this_index,
                        len(self.values), 'simulation needs finished')
                else:
                    #do nothing
                    print '%s [%3d/%3d] %s' % (
                        os.path.relpath(this_sim, work), this_index,
                        len(self.values), 'nothing to be done')
            else:
                if last_sim and os.path.exists(last_sim):
                    #use last sim
                    this_chk = lm.checkpoint.load_last(last_sim)
                    this_chk.reset(keep_elecs=True, keep_holes=True)
                    this_chk['iterations.real' ] = opts.equil
                    this_chk['iterations.print'] = opts.iprint
                    this_chk['seed.charges'    ] = 0.0
                    this_chk[self.working      ] = this_value
                    this_path = os.path.join(this_sim, 'part.0')
                    os.makedirs(this_path)
                    this_path = os.path.join(this_path, 'sim.inp')
                    this_chk.save(this_path)
                    print '%s [%3d/%3d] %s' % (
                        os.path.relpath(this_sim, work), this_index,
                        len(self.values), 'equil simulation')
                else:
                    #start fresh
                    this_chk = self.checkpoint
                    this_chk.reset(keep_elecs=True, keep_holes=True)
                    this_chk['iterations.real' ] = opts.fresh
                    this_chk['iterations.print'] = opts.iprint
                    this_chk[self.working      ] = this_value
                    this_path = os.path.join(this_sim, 'part.0')
                    os.makedirs(this_path)
                    this_path = os.path.join(this_path, 'sim.inp')
                    this_chk.save(this_path)
                    print '%s [%3d/%3d] %s' % (
                        os.path.relpath(this_sim, work), this_index,
                        len(self.values), 'fresh simulation')

            if this_chk:
                self.run(this_path)

    @staticmethod
    def run(input_file):
        """
        Run Langmuir on the input file and gzip the output
        """
        if not os.path.exists(input_file):
            raise RuntimeError('can not run Langmuir, %s does not exist'
                                   % input_file)
        input_file = os.path.abspath(input_file)
        current_directory = os.getcwd()
        working_directory = os.path.dirname(input_file)
        os.chdir(working_directory)
        check_call(['langmuir', input_file])
        check_call(['gzip', '-v'] + glob.glob('*'))
        os.chdir(current_directory)

if __name__ == '__main__':

    opts = get_arguments()
    work = os.getcwd()

    scanner = Scanner(opts)

    if opts.mode == 'scan':
        scanner.scan(opts)

    elif opts.mode == 'gen':
        scanner.generate(opts)

    else:
        raise RuntimeError('mode not supported : %s\n' % opts.mode +
            'set the mode with the --mode command line option')

    sys.exit(0)