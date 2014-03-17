# -*- coding: utf-8 -*-
"""
solar.py
========

.. argparse::
    :module: solar
    :func: create_parser
    :prog: solar.py

.. moduleauthor:: Adam Gagorik <adam.gagorik@gmail.com>
"""
import langmuir as lm
import argparse
import os

desc = """
Script to analyze solar cell IV curve.
"""

def create_parser():
    parser = argparse.ArgumentParser()
    parser.description = desc

    #input
    parser.add_argument(dest='pkls', nargs='*', help='input files')
    parser.add_argument('-r', default=False, action='store_true',
                        help='search recursivly for files')

    #output
    parser.add_argument('--stub', default='', type=str,
                        metavar='stub', help='output file stub')
    parser.add_argument('--csv', action='store_true', help='save CSV file')

    #fit
    choices=['tanh', 'power', 'interp1d', 'erf']
    parser.add_argument('--mode', choices=choices, type=str, metavar='str',
        help='fit method', default='interp1d')

    parser.add_argument('--shift', type=float, default=1.5, metavar='float',
        help='voltage shift')

    parser.add_argument('--recycle', action='store_true', default=False,
        help='recycle current-opt paramters as initial guess for power fit')

    #power
    parser.add_argument('--order', type=int, default=8, metavar='int',
        help='order of power series fit')

    #interp1d
    choices=['linear', 'nearest', 'zero', 'slinear', 'quadratic', 'cubic']
    parser.add_argument('--kind', type=str, choices=choices, default='cubic',
        help='interpolation type for interp1d', metavar='str')

    #plot
    parser.add_argument('--show', action='store_true', default=False,
                        help='open matplotlib window')
    parser.add_argument('--plot', action='store_true', default=False,
                        help='plot iv curves')
    return parser

def get_arguments(args=None):
    parser = create_parser()
    opts = parser.parse_args(args)

    if opts.show:
        opts.plot = True

    if not opts.pkls:
        work = os.getcwd()
        opts.pkls = lm.find.pkls(work, stub='gathered*', r=opts.r,
            at_least_one=True)

    assert opts.pkls

    parser.print_help()
    fmt = '  %-10s: %s'
    print '\nparameters:'
    print fmt % ('--pkls'   , len(opts.pkls))
    print fmt % ('--mode'   , opts.mode)
    print fmt % ('--shift'  , opts.shift)
    print fmt % ('--order'  , opts.order)
    print fmt % ('--kind'   , opts.kind)
    print fmt % ('--recycle', opts.recycle)
    print fmt % ('--stub'   , opts.stub)
    print fmt % ('--plot'   , opts.plot)
    print fmt % ('--show'   , opts.show)
    print ''

    print '\n'.join(opts.pkls)
    print ''

    return opts

class parm:
    def __init__(self):
        pass

    vlabel = r'V $\mathtt{(V)}$'

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    iv = lm.ivcurve.IVCurveSolar.load_pkls(opts.pkls)
    iv.calculate(mode=opts.mode, recycle=opts.recycle, order=opts.order,
        kind=opts.kind, k=opts.order)
    print iv

    handle = lm.common.format_output(ext='pkl', name='solar', stub=opts.stub)
    iv.save_pkl(handle)
    print 'saved:', handle

    if opts.csv:
        handle = lm.common.format_output(ext='csv', name='solar',
            stub=opts.stub)
        handle = open(handle, 'w')
        iv.save_csv(handle)
        print 'saved:', handle.name

    if opts.plot:
        import matplotlib.pyplot as plt

        fig, ax1 = lm.plot.subplots(l=1.75, r=0.75)

        lm.plot.rectangle(0, 0, iv.v_oc, iv.i_sc, fc=lm.plot.colors.y1)
        lm.plot.rectangle(0, 0, iv.v_mp, iv.i_mp, fc='w', alpha=1)

        lm.plot.errorbar(iv.v, iv.i, yerr=iv.ierr, color='b', lw=0)
        iv.ifit.plot(iv.stats.v.min, iv.stats.v.max, color='b', ls='-')

        plt.axhline(0, color='k')
        plt.axvline(0, color='k')

        plt.xlim(iv.stats.v.min, iv.stats.v.max)
        plt.ylim(iv.stats.i.min, iv.stats.i.max)
        lm.plot.zoom(l=0, r=0)

        lm.plot.maxn_locator(s=5)
        plt.xlabel(r'$V\,$($V$ )')
        plt.ylabel(r'$I\,(nA)$')

        kwargs = dict(ha='center', va='center', size='xx-small')
        plt.text(0.5 * iv.v_mp, 0.5 * iv.i_mp, '%.2f%%' % iv.fill, **kwargs)

        handle = lm.common.format_output(ext='pdf', name='solar',
            stub=opts.stub)
        lm.plot.save(handle)
        print 'saved:', handle

        if opts.show:
            plt.show()