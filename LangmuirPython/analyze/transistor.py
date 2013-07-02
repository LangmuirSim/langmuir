# -*- coding: utf-8 -*-
import matplotlib.pyplot as plt
import argparse
import langmuir
import os

def get_arguments(args=None):
    """
    Get command line arguments using :py:mod:`argparse`.

    :param args: override command line arguments
    :type args: list

    returns: :py:class:`argparse.Namespace`, option object
    """
    parser = argparse.ArgumentParser()
    parser.add_argument(dest='pkls', default=None, nargs='*', type=str,
                        help='input files')
    parser.add_argument('--work', default=os.getcwd(), type=str,
                        help='working directory')
    parser.add_argument('--show', default=False, action='store_true',
                        help='open matplotlib window')
    parser.add_argument('--fit', default=[], nargs='*',
                        help='linear fit slice')
    parser.add_argument('--csv', default=False, action='store_true',
                        help='save data to csv file')
    parser.add_argument('--pdf', default=False, action='store_true',
                        help='save plot to pdf')
    parser.add_argument('--png', default=False, action='store_true',
                        help='save plot to png')
    parser.add_argument('--pkl', default=False, action='store_true',
                        help='save plot to pkl')
    parser.add_argument('--stub', default='plot', type=str,
                        help='output file name stub')
    parser.add_argument('--title', default='', help='plot title')
    parser.add_argument('--color', default='b', help='line color')
    parser.add_argument('--marker', default='o', help='point marker')
    opts = parser.parse_args(args)

    assert len(opts.fit) % 2 == 0
    opts.fit = [langmuir.common.evaluate(s) for s in opts.fit]

    if not opts.pkl and not opts.csv and not opts.pdf and not opts.png:
        opts.show = True

    if not opts.pkls:
        opts.pkls = langmuir.find.pkls(opts.work, 'gathered*', 1, 1, 1)

    return opts

if __name__ == '__main__':
    opts = get_arguments()
    ivline = langmuir.ivline.IVLineT()
    ivline.load_pkls(opts.pkls)
    ivline.calculate()

    xmin = float(min(0, ivline.vstats.min))
    xmax = float(max(0, ivline.vstats.max))
    ymin = float(min(0, ivline.istats.min))
    ymax = float(max(0, ivline.istats.max))

    ivplot = langmuir.ivline.IVPlotT(xmin, xmax, ymin, ymax)

    for i in range(0, len(opts.fit), 2):
        s = slice(opts.fit[i], opts.fit[i + 1])
        f = ivline.fit(s)
        f.plot(xmin, xmax, label=f.mlabel + r' $\mathtt{nS}$')

    if ivline.fits:
        ivplot.legend(loc='best')

    ivplot.errorbar(ivline.v, ivline.i, yerr=ivline.ierr, ls='-',
                    color=opts.color, marker=opts.marker)

    ivplot.title(opts.title)
    ivplot.finish()

    if ymin == 0:
        langmuir.plot.zoom(l=0, r=0, b=0)
    if ymax == 0:
        langmuir.plot.zoom(l=0, r=0, t=0)

    if opts.csv:
        oname = '%s.csv' % opts.stub
        print 'saving %s...' % oname
        ivline.csv(oname)

    if opts.pkl:
        oname = '%s.pkl' % opts.stub
        print 'saving %s...' % oname
        ivline.pkl(oname)

    if opts.pdf:
        oname = '%s.pdf' % opts.stub
        print 'saving %s...' % oname
        plt.savefig(oname)

    if opts.png:
        oname = '%s.png' % opts.stub
        print 'saving %s...' % oname
        plt.savefig(oname)

    if opts.show:
        plt.show()

