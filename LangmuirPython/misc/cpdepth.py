# -*- coding: utf-8 -*-
"""
Created on Mon Jan  7 08:01:31 2013

@author: adam
"""

import argparse
import shutil
import glob
import sys
import os

def get_arguments(args=None):
    parser = argparse.ArgumentParser(description='depth sensitive recursive copy')
    parser.add_argument(dest='src', type=str, help='source path')
    parser.add_argument(dest='dest', type=str, help='destination path')
    parser.add_argument('--exclude', type=str, help='exclude patterns', nargs='*')
    parser.add_argument('--keep', type=str, help='keep patterns', nargs='*')
    parser.add_argument('--depth', required=True, metavar='int', type=int,
                        help='copy depth')
    parser.add_argument('--ignore', default=False, action='store_true',
                        help='do not check if paths exist already')
    opts = parser.parse_args(args)
    return parser, opts

if __name__ == '__main__':
    parser, opts = get_arguments()
    work = os.getcwd()

    pattern = opts.src
    for i in range(opts.depth):
        pattern = os.path.join(pattern, '*')

    paths = [os.path.relpath(item, work)
                 for item in glob.glob(pattern)
                     if os.path.isdir(item)]
    found = []
    for path in paths:
        root, dirs, files = os.walk(path).next()
        files = [os.path.join(root, f) for f in files]

        if not opts.exclude is None:
            for exclude_pattern in opts.exclude:
                exclude = glob.glob(os.path.join(root, exclude_pattern))
                files = [f for f in files if not f in exclude]

        if not opts.keep is None:
            kept = []
            for keep_pattern in opts.keep:
                keep = glob.glob(os.path.join(root, keep_pattern))
                to_keep = [f for f in files if f in keep]
                kept.extend(to_keep)
            files = kept

        found.extend(files)

        print '[%s]' % os.path.relpath(path, opts.src)
        for f in files:
            print '\t%s' % os.path.relpath(f, path)
        print ''

    print 'dest = [%s]' % opts.dest

    if not found:
        print '\nerror: no files found'
        print '\ncopy aborted'
        sys.exit(-1)

    if not opts.ignore:
        for path in paths:
            outpath = os.path.join(opts.dest, path)
            if os.path.exists(outpath):
                print '\nerror: output path already exists\n\t%s' % outpath
                print '\ncopy aborted'
                sys.exit(-1)

    answer = raw_input('\nproceed with copy? (y/n): ')

    if answer.lower() in ['y', 'yes']:
        print '\ncopying files...'
        for f in found:
            new = os.path.join(opts.dest, os.path.dirname(os.path.relpath(f, work)))
            if not os.path.exists(new):
                os.makedirs(new)
            shutil.copy(f, new)
        print 'done'
    else:
        print '\ncopy aborted'
