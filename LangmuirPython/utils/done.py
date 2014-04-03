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
import sys
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

def indent(string, level=0, **kwargs):
    print '    ' * level + string.format(**kwargs)

def report_chk(chk):
    chk = lm.checkpoint.load(chk)
    try:
        cstep = chk['current.step']
    except KeyError:
        cstep = '?'

    try:
        tstep = chk['iterations.real']
    except KeyError:
        tstep = '?'

    try:
        percent = float(cstep)/float(tstep) * 100
    except TypeError:
        percent = 0.00

    done = False
    if percent >= 100:
        done=True

    percent = '%.2f' % percent

    return cstep, tstep, percent, done

format_string = '{path} : cstep={cstep:>8} : tstep={tstep:>8} : percent={percent:>5} : done={done}'

def loop_part(part, level=0):
    chk = lm.find.chk(part)
    cstep, tstep, percent, done = report_chk(chk)
    indent(format_string, level=level, path=os.path.basename(part), cstep=cstep, tstep=tstep, percent=percent,
           done=done)

def loop_sim(sim, level=0):
    indent('sim: {sim}', level=level, sim=os.path.basename(sim))
    parts = lm.find.parts(sim)
    for part in parts:
        loop_part(part, level=level + 1)

def loop_run(run, level=0):
    indent('run: {run}', level=level, run=os.path.basename(run))
    sims = lm.find.sims(run)
    for sim in sims:
        loop_sim(sim, level=level + 1)

def loop_system(system, level=0):
    indent('system: {system}', level=level, system=os.path.basename(system))
    runs = lm.find.runs(system)
    for run in runs:
        loop_run(run, level=level + 1)

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    systems = lm.find.systems(work, r=opts.r)
    if not systems:
        runs = lm.find.runs(work, r=opts.r)
        if not runs:
            sims = lm.find.sims(work, r=opts.r)
            if not sims:
                parts = lm.find.parts(work, r=opts.r)
                if not parts:
                    chks = lm.find.chks(work, r=opts.r)
                    if not chks:
                        dats = lm.find.dats(work, r=opts.r)
                        if not dats:
                            print 'can not find any simulations!'
                            sys.exit(-1)
                    else:
                        for chk in chks:
                            cstep, tstep, percent, done = report_chk(chk)
                            indent(format_string, level=0, path=os.path.basename(chk), cstep=cstep,
                                tstep=tstep, percent=percent, done=done)
                else:
                    for part in parts:
                        loop_part(part)
            else:
                for sim in sims:
                    loop_sim(sim)
        else:
            for run in runs:
                loop_run(run)
    else:
        for system in systems:
            loop_system(system)