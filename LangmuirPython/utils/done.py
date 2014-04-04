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
from langmuir.common import ccodes
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
    parser.add_argument('-d', action='store_true', help='show which simulations are not done')
    return parser

def get_arguments(args=None):
    parser = create_parser()
    opts = parser.parse_args(args)
    return opts

def indent(string, level=0, **kwargs):
    print '    ' * level + string.format(ccodes=ccodes, **kwargs)

format_string = '(' + ccodes.f('path'   ) + '='  + ccodes.y('{path}'   ) + ') : ' + \
                '(' + ccodes.f('cstep'  ) + '='  + ccodes.c('{cstep}'  ) + ') : ' + \
                '(' + ccodes.f('tstep'  ) + '='  + ccodes.c('{tstep}'  ) + ') : ' + \
                '(' + ccodes.f('percent') + '='  + ccodes.c('{percent}') + ') : ' + \
                '(' + ccodes.f('done'   ) + '='  +          '{done}'     + ')'

def get_cstep(obj):
    try:
        cstep = obj['current.step']
    except:
        cstep = None
    return cstep

def get_tstep(obj):
    try:
        tstep = obj['iterations.real']
    except:
        tstep = None
    return tstep

def get_percent(cstep, tstep):
    try:
        percent = float(cstep)/float(tstep) * 100
    except:
        percent = 0
    return percent, percent >= 100

def analyze_chk(chk):
    if chk:
        chk = lm.checkpoint.load(chk)

    cstep = get_cstep(chk)
    tstep = get_tstep(chk)

    return cstep, tstep

def analyze_dat(dat):
    try:
        return int(lm.common.tail(dat).strip().split()[0])
    except:
        return None

def loop_part(part, level=0, record=True):
    chk = lm.find.chk(part)
    cstep, tstep = analyze_chk(chk)

    if not tstep:
        inp = lm.find.inp(part)
        tstep = get_tstep(inp)

    if not tstep:
        parm = lm.find.parm(part)
        tstep = get_tstep(parm)

    if not cstep:
        dat = lm.find.dat(part)
        cstep = analyze_dat(dat)

    percent, done = get_percent(cstep, tstep)

    if not tstep:
        tstep = '?'

    if not cstep:
        cstep = '?'

    if not done:
        if record:
            results['not_done'].append(part)
        done = ccodes.red('%s' % done)
    else:
        if record:
            results['done'].append(part)
        done = ccodes.green('%s' % done)

    indent(format_string, level=level, path=os.path.basename(part), cstep=cstep,
           tstep=tstep, percent=percent, done=done)

def loop_sim(sim, level=0):
    title = ccodes.f('sim') + '=' + ccodes.blue('{sim}')
    indent(title, level=level, sim=os.path.basename(sim))
    parts = lm.find.parts(sim)
    for part in parts:
        loop_part(part, level=level + 1)

def loop_run(run, level=0):
    title = ccodes.f('run') + '=' + ccodes.magenta('{run}')
    indent(title, level=level, run=os.path.basename(run))
    sims = lm.find.sims(run)
    for sim in sims:
        loop_sim(sim, level=level + 1)

def loop_system(system, level=0):
    title = ccodes.f('system') + '=' + ccodes.cyan('{system}')
    indent(title, level=level, system=os.path.basename(system))
    runs = lm.find.runs(system)
    for run in runs:
        loop_run(run, level=level + 1)

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    results = {
            'done' : [],
        'not_done' : []
    }

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
                            inps = lm.find.inps(work, r=opts.r)
                            if not inps:
                                for inp in inps:
                                    loop_part(os.path.dirname(inp))
                            else:
                                print 'can not find any simulations!'
                                sys.exit(-1)
                        else:
                            for dat in dats:
                                loop_part(os.path.dirname(dat))
                    else:
                        for chk in chks:
                            loop_part(os.path.dirname(chk))
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

    if opts.d:
        print ''
        for not_done in results['not_done']:
            loop_part(not_done, record=False)

    print ''
    print '(' + ccodes.f('done'    ) + '=' + ccodes.g('%d' % len(results['done'    ])) + ')' + ' : '  + \
          '(' + ccodes.f('not done') + '=' + ccodes.r('%d' % len(results['not_done'])) + ')'