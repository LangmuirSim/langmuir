# -*- coding: utf-8 -*-
"""
stats.py
========

.. argparse::
    :module: stats
    :func: create_parser
    :prog: stats.py

.. moduleauthor:: Adam Gagorik <adam.gagorik@gmail.com>
"""
import scipy.stats as stats
import langmuir as lm
import numpy as np
import collections
import argparse
import StringIO
import os

desc = """
Compute avg, std, min, max, rng, corrcoeffs, skew, kurtosis for data sets.
"""

def create_parser():
    parser = argparse.ArgumentParser()
    parser.description = desc

    parser.add_argument(dest='inputA', type=str, metavar='input',
        help='input file 1')

    parser.add_argument(dest='inputB', type=str, default=None, nargs='?',
        metavar='input', help='input file 2')

    parser.add_argument('--stub', default='', type=str, metavar='stub',
        help='output file stub')

    return parser

def get_arguments(args=None):
    parser = create_parser()
    opts = parser.parse_args(args)

    if opts.inputB is None:
        opts.inputB = opts.inputA

    return opts

def fix_value(value, tol=1e-99):
    if value < tol:
        return 0.0
    return value

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    x = lm.surface.load(opts.inputA)
    y = lm.surface.load(opts.inputB)

    x_avg = np.mean(x)
    y_avg = np.mean(y)

    x_std = np.std(x)
    y_std = np.std(y)

    x_min = np.amin(x)
    y_min = np.amin(y)

    x_max = np.amax(x)
    y_max = np.amax(y)

    x_rng = abs(x_max - x_min)
    y_rng = abs(y_max - y_min)

    x_del = x - x_avg
    y_del = y - y_avg

    corr_xx = x_del * x_del / (x_std * x_std)
    corr_yy = y_del * y_del / (y_std * y_std)
    corr_xy = x_del * y_del / (x_std * y_std)

    cov_matrix = np.corrcoef(x.flat, y.flat)
    r2_xx = cov_matrix[0,0]
    r2_xy = cov_matrix[0,1]
    r2_yx = cov_matrix[1,0]
    r2_yy = cov_matrix[1,1]

    skew_x = stats.skew(x.flat, 0, bias=False)
    skew_y = stats.skew(y.flat, 0, bias=False)

    skew_xz, skew_xp = stats.skewtest(x.flat, 0)
    skew_yz, skew_yp = stats.skewtest(y.flat, 0)

    kurtosis_x = stats.kurtosis(x.flat, 0, bias=False)
    kurtosis_y = stats.kurtosis(y.flat, 0, bias=False)

    kurtosis_xz, kurtosis_xp = stats.kurtosistest(x.flat, 0)
    kurtosis_yz, kurtosis_yp = stats.kurtosistest(y.flat, 0)

    results = collections.OrderedDict()

    results['x_path'] = os.path.basename(opts.inputA)
    results['y_path'] = os.path.basename(opts.inputB)

    results['x'] = x
    results['y'] = y

    results['x_avg'] = x_avg
    results['y_avg'] = y_avg

    results['x_std'] = x_std
    results['y_std'] = y_std

    results['x_min'] = x_min
    results['y_min'] = y_min

    results['x_max'] = x_max
    results['y_max'] = y_max

    results['x_rng'] = x_rng
    results['y_rng'] = y_rng

    results['corr_xx'] = corr_xx
    results['corr_yy'] = corr_yy
    results['corr_xy'] = corr_xy

    results['r2_xx'] = r2_xx
    results['r2_yy'] = r2_yy
    results['r2_xy'] = r2_xy
    results['r2_yx'] = r2_yx

    results['skew_x'] = skew_x
    results['skew_y'] = skew_y

    results['skew_xz'] = skew_xz
    results['skew_yz'] = skew_yz

    results['skew_xp'] = skew_xp
    results['skew_yp'] = skew_yp

    results['kurtosis_x'] = kurtosis_x
    results['kurtosis_y'] = kurtosis_y

    results['kurtosis_xz'] = kurtosis_xz
    results['kurtosis_yz'] = kurtosis_yz

    results['kurtosis_xp'] = kurtosis_xp
    results['kurtosis_yp'] = kurtosis_yp

    fmt = StringIO.StringIO()
    print >> fmt, '[Results]'
    print >> fmt, '  x       {x_path}'
    print >> fmt, '  y       {y_path}'
    print >> fmt, ''
    print >> fmt, '  x_avg   {x_avg:{w}.{p}e}'
    print >> fmt, '  x_std   {x_std:{w}.{p}e}'
    print >> fmt, '  x_min   {x_min:{w}.{p}e}'
    print >> fmt, '  x_max   {x_max:{w}.{p}e}'
    print >> fmt, '  x_rng   {x_rng:{w}.{p}e}'
    print >> fmt, ''
    print >> fmt, '  y_avg   {y_avg:{w}.{p}e}'
    print >> fmt, '  y_std   {y_std:{w}.{p}e}'
    print >> fmt, '  y_min   {y_min:{w}.{p}e}'
    print >> fmt, '  y_max   {y_max:{w}.{p}e}'
    print >> fmt, '  y_rng   {y_rng:{w}.{p}e}'
    print >> fmt, ''
    print >> fmt, '  r2_xx   {r2_xx:{w}.{p}g}'
    print >> fmt, '  r2_yy   {r2_yy:{w}.{p}g}'
    print >> fmt, '  r2_xy   {r2_xy:{w}.{p}g}'
    print >> fmt, '  r2_yx   {r2_yx:{w}.{p}g}'
    print >> fmt, ''
    print >> fmt, '  skew_x  {skew_x:{w}.{p}f}'
    print >> fmt, '  pvalue  {skew_xp:{w}.{p}f}'
    print >> fmt, '  zvalue  {skew_xz:{w}.{p}f}'
    print >> fmt, ''
    print >> fmt, '  skew_y  {skew_y:{w}.{p}f}'
    print >> fmt, '  pvalue  {skew_yp:{w}.{p}f}'
    print >> fmt, '  zvalue  {skew_yz:{w}.{p}f}'
    print >> fmt, ''
    print >> fmt, '  kurr_x  {kurtosis_x:{w}.{p}f}'
    print >> fmt, '  pvalue  {kurtosis_xp:{w}.{p}f}'
    print >> fmt, '  zvalue  {kurtosis_xz:{w}.{p}f}'
    print >> fmt, ''
    print >> fmt, '  kurr_y  {kurtosis_y:{w}.{p}f}'
    print >> fmt, '  pvalue  {kurtosis_yp:{w}.{p}f}'
    print >> fmt, '  zvalue  {kurtosis_yz:{w}.{p}f}'

    print fmt.getvalue().format(w=15, p=8, **results)

    handle = lm.common.format_output(stub=opts.stub, name='stats', ext='pkl')
    print 'saved: %s' % handle
    lm.common.save_pkl(results, handle)

    handle = lm.common.format_output(stub=opts.stub, name='stats', ext='dat')
    with open(handle, 'w') as ohandle:
        ohandle.write(fmt.getvalue().format(w=15, p=8, **results))