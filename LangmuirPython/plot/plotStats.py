# -*- coding: utf-8 -*-
"""
@author: adam
"""
from subprocess import check_call, CalledProcessError
import langmuir as lm
import argparse
import tempfile
import shutil
import os
import re

desc = """
Plot output from rdf.py
"""

def get_arguments(args=None):
    parser = argparse.ArgumentParser()
    parser.description = desc

    parser.add_argument(dest='ifile', default='stats.pkl', type=str, nargs='?',
                        metavar='input', help='input file')
    parser.add_argument('--stub', default='', type=str, metavar='stub',
                        help='output file stub')
    parser.add_argument('--compile', action='store_true', help='run latex')
    parser.add_argument('--keep', action='store_true', help='keep tempdir')

    opts = parser.parse_args(args)

    return opts

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    data = lm.common.load_pkl(opts.ifile)

    handle = lm.common.format_output(stub=opts.stub, name='stats', ext='tex')
    handle = open(handle, 'w')

    print >> handle, r'\documentclass{article}'
    print >> handle, r'\usepackage[margin=1.5cm]{geometry}'
    print >> handle, r'\usepackage{graphicx}'
    print >> handle, r'\usepackage{tabu}'
    print >> handle, r'\begin{document}'
    print >> handle, r'\thispagestyle{empty}'
    print >> handle, r'\begin{tabu} to 4in {X[-1rm]|[1pt]X[-1rm]|[1pt]X[-1rm]}'
    print >> handle, r'\rowfont[r]{\small\bfseries}'
    print >> handle, r'& {x_path} & {y_path} \\'.format(**data)
    print >> handle, r'\tabucline[1pt]{}'
    print >> handle, r'{name} & ${x_avg:.5e}$ & ${y_avg:.5e}$ \\'.format(name='avg', **data)
    print >> handle, r'{name} & ${x_std:.5e}$ & ${y_std:.5e}$ \\'.format(name='std', **data)
    print >> handle, r'{name} & ${x_min:.5e}$ & ${y_min:.5e}$ \\'.format(name='min', **data)
    print >> handle, r'{name} & ${x_max:.5e}$ & ${y_max:.5e}$ \\'.format(name='max', **data)
    print >> handle, r'{name} & ${x_rng:.5e}$ & ${y_rng:.5e}$ \\'.format(name='rng', **data)   
    print >> handle, r'\tabucline[1pt]{}'
    print >> handle, r'{name} & ${skew_x:.5e} $ & ${skew_y:.5e} $ \\'.format(name='skew', **data)    
    print >> handle, r'{name} & ${skew_xp:.5e}$ & ${skew_yp:.5e}$ \\'.format(name='pvalue', **data)    
    print >> handle, r'{name} & ${skew_xz:.5e}$ & ${skew_yz:.5e}$ \\'.format(name='zvalue', **data)    
    print >> handle, r'\tabucline[1pt]{}'
    print >> handle, r'{name} & ${kurtosis_x:.5e} $ & ${kurtosis_y:.5e} $ \\'.format(name='kurtosis', **data)    
    print >> handle, r'{name} & ${kurtosis_xp:.5e}$ & ${kurtosis_yp:.5e}$ \\'.format(name='pvalue', **data)    
    print >> handle, r'{name} & ${kurtosis_xz:.5e}$ & ${kurtosis_yz:.5e}$ \\'.format(name='zvalue', **data)    
    print >> handle, r'\tabucline[1pt]{}'
    print >> handle, r'{name} & $xx={r2_xx:.5e}$ & $xy={r2_xy:.5e}$ \\'.format(name='$R^{2}$', **data)
    print >> handle, r'{name} & $yx={r2_yx:.5e}$ & $yy={r2_yy:.5e}$ \\'.format(name='', **data) 
    print >> handle, r'\end{tabu}'
    print >> handle, r'\end{document}'

    handle.close()
    print 'saved: %s' % handle.name
    
    if opts.compile:
        path = tempfile.mkdtemp(dir=work)
        shutil.copy(handle.name, path)
        
        print path
        os.chdir(path)
        
        try:
            check_call('latexmk -pdf %s' % handle.name, shell=True)
        except CalledProcessError as e:
            print e
            print 'CAN NOT COMPILE LATEX FILE'
        
        pdf = os.path.join(path, re.sub('.tex', '.pdf', handle.name))
        if os.path.exists(pdf):
            lm.plot.crop(pdf, border=0)
            shutil.copy(pdf, work)
        
        if os.path.exists(path) and not opts.keep:
            shutil.rmtree(path)