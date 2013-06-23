# -*- coding: utf-8 -*-
"""
Created on Fri Apr  5 08:07:53 2013

@author: adam
"""

import langmuir
import argparse
import sys
import os

parser = argparse.ArgumentParser()
parser.description = 'find simulations in directory'
parser.add_argument('--percent', action='store_true', default=False,
                    help='find percent done')
parser.add_argument('--time', action='store_true', default=False,
                    help='find simulation time')
opts = parser.parse_args()

work = os.getcwd()

print 'searching for systems...'
systems = langmuir.find.systems(work)
if not systems:
    print 'no systems found!\n'
    print 'searching for runs...'
    runs = langmuir.find.runs(work, recursive=True)
    if not runs:
        print 'no runs found!\n'
        print 'searching for parts...'
        sims = langmuir.find.sims(work, stub='voltage*', recursive=True)
        if not sims:
            print 'no sims found!\n'
            print 'searching for parts...'
            parts = langmuir.find.parts(work, recursive=True)
            if not parts:
                print 'no parts found!\n'
                print 'searching for dat and chk...'
                dat = langmuir.find.dat(work)
                chk = langmuir.find.chk(work)
                if dat and chk:
                    part = langmuir.find.Part(work)
                    if opts.percent:
                        part.get_percent()
                    if opts.time:
                        part.get_time()
                    print 'found dat and chk!\n'
                    print part
                else:
                    print 'no dat and/or chk found!'
                    sys.exit(-1)
            else:
                sim = langmuir.find.Sim(work)
                print 'found parts!\n'
                print sim
                for part in sim:
                    if opts.percent:
                        part.get_percent()
                    if opts.time:
                        part.get_time()
                    print ' %s' % part
        else:
            run = langmuir.find.Run(work)
            print 'found sims!\n'
            print run
            for sim in run:
                print ' %s' % sim
                for part in sim:
                    if opts.percent:
                        part.get_percent()
                    if opts.time:
                        part.get_time()
                    print '  %s' % part
                print ''
    else:
        system = langmuir.find.System(work)
        print 'found runs!\n'
        print system
        for run in system:
            print ' %s' % run
            for sim in run:
                print '  %s' % sim
                for part in sim:
                    if opts.percent:
                        part.get_percent()
                    if opts.time:
                        part.get_time()
                    print '   %s' % part
                print ''
else:
    print 'found systems!\n'
    for system in systems:
        system = langmuir.find.System(system)
        print system
        for run in system:
            print ' %s' % run
            for sim in run:
                print '  %s' % sim
                for part in sim:
                    if opts.percent:
                        part.get_percent()
                    if opts.time:
                        part.get_time()
                    print '   %s' % part
                print ''
