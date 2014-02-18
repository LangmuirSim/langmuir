# -*- coding: utf-8 -*-
"""
@author: adam
"""
import langmuir as lm
import argparse
import os

desc = """
Compare two checkpoint files.
"""

def get_arguments(args=None):
    parser = argparse.ArgumentParser()
    parser.description = desc
    parser.add_argument(dest='chk1', default=None, type=str, metavar='chk1',
                        help='checkpoint file 1')
    parser.add_argument(dest='chk2', default=None, type=str, metavar='chk1',
                        help='checkpoint file 2')
    parser.add_argument('--reset', default=False, action='store_true',
                        help='reset checkpoint files first')
    opts = parser.parse_args(args)
    return opts

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    chk1 = lm.checkpoint.load(opts.chk1)
    chk2 = lm.checkpoint.load(opts.chk2)

    if opts.reset:
        chk1.reset()
        chk2.reset()

    results = lm.checkpoint.compare(chk1, chk2)
    valid   = results['valid']

    if not valid:
        for k1, r1 in results.iteritems():
            try:
                if not r1['valid']:
                    print k1
                    print 39 * '*'
                    for k2, r2 in r1.iteritems():
                        try:
                            print '%-20s %-10s %-10s' % (k2, r2[0], r2[1])
                        except:
                            print '%-20s %-10s' % (k2, r2)
                    print ''
            except:
                pass

    print 'chk1: %s' % os.path.abspath(opts.chk1)
    print 'chk2: %s' % os.path.abspath(opts.chk2)
    print 'same: %s' % valid