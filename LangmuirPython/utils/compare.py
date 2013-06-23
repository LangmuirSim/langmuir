# -*- coding: utf-8 -*-
"""
.. module:: compare
    :platform: Unix
    :synopsis: Compare two checkpoint files.

.. moduleauthor:: Adam Gagorik <adam.gagorik@gmail.com>
"""
import argparse
import langmuir
import sys
import os

def get_arguments(args=None):
    """
    Get command line arguments using :py:mod:`argparse`.

    :param args: override command line arguments
    :type args: list

    returns: :py:class:`argparse.Namespace`, option object
    """
    parser = argparse.ArgumentParser()
    parser.add_argument(dest='chk1', type=str, default=None, nargs='?',
                        help='checkpoint file name 1 or directory name if -r')
    parser.add_argument(dest='chk2', help='checkpoint file name 2', type=str,
                        default=None, nargs='?')
    parser.add_argument('--reset', default=False, action='store_true',
                        help='reset checkpoint files first')
    parser.add_argument('-r', default=False, action='store_true',
                        help='recursivly compare checkpoints that are found')
    opts = parser.parse_args(args)

    if opts.r:
        if opts.chk1 is None:
            opts.chk1 = os.getcwd()
    else:
        if opts.chk1 is None or opts.chk2 is None:
            parser.print_help()
            sys.exit(-1)

    return opts

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    if opts.r:
        chks = langmuir.find.chks(opts.chk1, recursive=True)
        for i, chk_i in enumerate(chks):
            chk_i = langmuir.checkpoint.load(chk_i)
            if opts.reset:
                chk_i.reset()
            for j, chk_j in enumerate(chks):
                if j > i:
                    chk_j = langmuir.checkpoint.load(chk_j)
                    if opts.reset:
                        chk_j.reset()
                    same, results = langmuir.checkpoint.compare(chk_i, chk_j)
                    print 'chk1: %s' % os.path.relpath(chk_i.path, work)
                    print 'chk2: %s' % os.path.relpath(chk_j.path, work)
                    print 'same: %s' % same
                    if not same:
                        for k, r in results.iteritems():
                            if not r['valid']:
                                print '%s are different...' % k
                                for k, v in r.iteritems():
                                    print k, v
                    print ''
    else:
        chk1 = langmuir.checkpoint.load(opts.chk1)
        chk2 = langmuir.checkpoint.load(opts.chk2)

        if opts.reset:
            chk1.reset()
            chk2.reset()

        same, results = langmuir.checkpoint.compare(chk1, chk2)

        print 'chk1: %s' % os.path.relpath(chk1.path, work)
        print 'chk2: %s' % os.path.relpath(chk2.path, work)
        print 'same: %s' % same
        if not same:
            print ''
            for k, r in results.iteritems():
                if not r['valid']:
                    print '%s are different...' % k
                    for k, v in r.iteritems():
                        print k, v
                    print ''
