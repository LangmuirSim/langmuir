# -*- coding: utf-8 -*-
import matplotlib as mpl
mpl.rc('text', usetex=True)
mpl.rc('text.latex', preamble=[r'\usepackage{tabu}'])
import matplotlib.pyplot as plt
import langmuir
import argparse
import StringIO
import os

def get_arguments(args=None):
    """
    Get command line arguments using :py:mod:`argparse`.

    :param args: override command line arguments
    :type args: list

    returns: :py:class:`argparse.Namespace`, option object
    """
    parser = argparse.ArgumentParser()
    parser.add_argument(dest='pkls', nargs='*', help='input files')

    parser.add_argument('-r', default=False, action='store_true',
                        help='search recursivly for files')

    choices=['tanh', 'power', 'interp1d', 'erf']
    parser.add_argument('--mode', choices=choices, type=str, metavar='str',
        help='fit method', default='interp1d')

    parser.add_argument('--shift', type=float, default=1.5, metavar='float',
        help='voltage shift')

    parser.add_argument('--order', type=int, default=20, metavar='int',
        help='order of power series fit')

    choices=['linear', 'nearest', 'zero', 'slinear', 'quadratic', 'cubic']
    parser.add_argument('--kind', type=str, choices=choices, default='cubic',
        help='interpolation type for interp1d', metavar='str')

    parser.add_argument('--recycle', action='store_true', default=False,
        help='recycle current-opt paramters as initial guess for power fit')

    parser.add_argument('--stub', default='', type=str, metavar='str',
                        help='stub for output files')

    parser.add_argument('--show', action='store_true', default=False,
                        help='open matplotlib window')

    parser.add_argument('--plot', action='store_true', default=False,
                        help='plot iv curves')

    opts = parser.parse_args(args)

    if not opts.pkls:
        opts.work = os.getcwd()
        opts.pkls = langmuir.find.pkls(opts.work, 'gathered*',
                                       recursive=opts.r, at_least_one=True)

    elif len(opts.pkls) == 1 and os.path.isdir(opts.pkls[0]):
        opts.work = os.path.abspath(opts.pkls[0])
        opts.pkls = langmuir.find.pkls(opts.work, 'gathered*',
                                       recursive=opts.r, at_least_one=True)

    else:
        opts.work = os.getcwd()

    opts.stub = opts.stub.lstrip('-')

    if opts.show:
        opts.plot = True

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

    if opts.stub:
        opts.stub = '-%s' % opts.stub

    return opts

def mtext(string):
    return r'$\mathsf{%s}$' % string

class PlotParams(object):
    def __init__(self, **kwargs):
        self.xmin = None
        self.xmax = None
        self.ymin = None
        self.ymax = None

        self.title = ''
        self.fontsize_label = 'large'
        self.fontsize_table = 'medium'
        self.fontsize_title = 'large'
        self.fontsize_ticks = 'large'
        self.fontsize_other = 'medium'

        self.v_str = mtext(r'\ensuremath{V}')
        self.i_str = mtext(r'\ensuremath{nA}')
        self.p_str = mtext(r'\ensuremath{nW}')
        self.a_str = mtext(r'\ensuremath{nm^{-2}}')
        self.d_str = mtext(r'\ensuremath{mA\,cm^{-2}}')
        self.r_str = mtext(r'\ensuremath{mW\,cm^{-2}}')
        self.f_str = mtext(r'\ensuremath{\%}')

        self.vlabel = '%s (%s)' % (mtext('V'), self.v_str)
        self.ilabel = '%s (%s)' % (mtext('I'), self.i_str)
        self.plabel = '%s (%s)' % (mtext('P'), self.p_str)
        self.dlabel = '%s (%s)' % (mtext('J'), self.d_str)
        self.rlabel = '%s (%s)' % (mtext('R'), self.r_str)

        self.v_oc_str = mtext(r'\ensuremath{v_{oc}}')
        self.v_mp_str = mtext(r'\ensuremath{v_{mp}}')
        self.i_sc_str = mtext(r'\ensuremath{i_{sc}}')
        self.i_mp_str = mtext(r'\ensuremath{i_{mp}}')
        self.p_th_str = mtext(r'\ensuremath{p_{th}}')
        self.p_mp_str = mtext(r'\ensuremath{p_{mp}}')
        self.d_sc_str = mtext(r'\ensuremath{j_{sc}}')
        self.d_mp_str = mtext(r'\ensuremath{j_{mp}}')
        self.r_th_str = mtext(r'\ensuremath{p_{th}}')
        self.r_mp_str = mtext(r'\ensuremath{p_{mp}}')
        self.fill_str = mtext(r'\ensuremath{FF}')

        self.kws_subplots = dict(w=6.00, h=6.00, l=1.75, r=0.50, t=1.00,
            b=1.25, twinx=False)

        colors = langmuir.plot.colors
        icolor = colors['r1']
        pcolor = colors['b2']

        self.kws_idata = dict(color=icolor, marker='o', mfc=icolor, mec='k',
            label=mtext('current'))

        self.kws_pdata = dict(color=pcolor, marker='s', mfc=pcolor, mec='k',
            label=mtext('power'))

        self.kws_ifit = dict(color='k', ls='-', lw=1)
        self.kws_pfit = dict(color='k', ls='-', lw=1)

        self.kws_orect = dict(fc=colors['y1'], alpha=1)
        self.kws_irect = dict(fc='w', alpha=1)

        self.kws_axes = dict(color='k', lw=2)
        self.kws_span = dict(color='k', ls=':', lw=1)

        self.kws_legend = dict(borderaxespad=1, loc='lower right',
                               prop=dict(size=self.fontsize_other))

        vars(self).update(**kwargs)

def symmetric_ylimits(lim=None):
    if lim is None:
        ymin, ymax = plt.ylim()
        lim = max(abs(ymin), abs(ymax))
    plt.ylim(-abs(lim), abs(lim))

def ltable(parm, iv, vfmt='%.3f', ifmt='%.3f', pfmt='%.3f', ffmt='%.3f',
           density=False, x=0.05, y=0.95, fc='w', alpha=1, **kwargs):
    fdict = dict(parm=parm, iv=iv)
    fdict.update(fill=mtext(ffmt % float(iv.fill)))
    fdict.update(v_oc=mtext(vfmt % float(iv.v_oc)))
    fdict.update(v_mp=mtext(vfmt % float(iv.v_mp)))
    fdict.update(i_sc=mtext(ifmt % float(iv.i_sc)))
    fdict.update(i_mp=mtext(ifmt % float(iv.i_mp)))
    fdict.update(p_th=mtext(pfmt % float(iv.p_th)))
    fdict.update(p_mp=mtext(pfmt % float(iv.p_mp)))
    fdict.update(d_sc=mtext(ifmt % float(iv.d_sc)))
    fdict.update(d_mp=mtext(ifmt % float(iv.d_mp)))
    fdict.update(r_th=mtext(pfmt % float(iv.r_th)))
    fdict.update(r_mp=mtext(pfmt % float(iv.r_mp)))
    s = StringIO.StringIO()
    print >> s, r'\begin{{tabu}} spread 0pt {{X[-1lm]X[-1rm]X[-1lm]}}'
    print >> s, r'  \tabulinestyle{{2pt}}'
    print >> s, r'  {parm.fill_str} & {fill} & {parm.f_str} \\'
    print >> s, r'  {parm.v_oc_str} & {v_oc} & {parm.v_str} \\'
    print >> s, r'  {parm.v_mp_str} & {v_mp} & {parm.v_str} \\'
    if density:
        print >> s, r'  {parm.d_sc_str} & {d_sc} & {parm.d_str} \\'
        print >> s, r'  {parm.d_mp_str} & {d_mp} & {parm.d_str} \\'
        print >> s, r'  {parm.r_th_str} & {r_th} & {parm.r_str} \\'
        print >> s, r'  {parm.r_mp_str} & {d_mp} & {parm.r_str} \\'
    else:
        print >> s, r'  {parm.i_sc_str} & {i_sc} & {parm.i_str} \\'
        print >> s, r'  {parm.i_mp_str} & {i_mp} & {parm.i_str} \\'
        print >> s, r'  {parm.p_th_str} & {p_th} & {parm.p_str} \\'
        print >> s, r'  {parm.p_mp_str} & {p_mp} & {parm.p_str} \\'
    print >> s, r'\end{{tabu}}'
    s = ' '.join(s.getvalue().format(**fdict).strip().split('\n'))
    fc = langmuir.plot.fake_alpha(fc, alpha)
    _kwargs = dict(fontsize=parm.fontsize_table, va='top', ha='left',
                   zorder=1000, transform=plt.gca().transAxes,
                   bbox=dict(fc=fc, lw=2, pad=10))
    _kwargs.update(**kwargs)
    return plt.text(x, y, s, **_kwargs)

def plotIV():
    parm = PlotParams(xmin=-0.75, xmax=3.0)

    fig, ax1 = langmuir.plot.subplots(**parm.kws_subplots)

    langmuir.plot.rectangle(0, 0, iv.v_oc, iv.i_sc, **parm.kws_orect)
    langmuir.plot.rectangle(0, 0, iv.v_mp, iv.i_mp, **parm.kws_irect)

    plt.axhline(0, **parm.kws_axes)
    plt.axvline(0, **parm.kws_axes)

    plt.axhline(iv.i_sc, **parm.kws_span)
    plt.axhline(iv.i_mp, **parm.kws_span)
    plt.axvline(iv.v_oc, **parm.kws_span)
    plt.axvline(iv.v_mp, **parm.kws_span)

    plt.errorbar(iv.v, iv.i, yerr=iv.ierr, **parm.kws_idata)
    plt.errorbar(iv.v, iv.p, yerr=iv.perr, **parm.kws_pdata)

    iv.ifit.plot(parm.xmin, parm.xmax, **parm.kws_ifit)
    iv.pfit.plot(parm.xmin, parm.xmax, **parm.kws_pfit)

    ltable(parm, iv, density=False)
    plt.legend(**parm.kws_legend)

    plt.xlim(parm.xmin, parm.xmax)

    plt.ylim(iv.istats.min, iv.istats.max)
    langmuir.plot.zoom(l=0, r=0, factor=0.25)
    symmetric_ylimits()

    plt.title(parm.title, fontsize=parm.fontsize_title)
    plt.xlabel(parm.vlabel, fontsize=parm.fontsize_label)
    plt.ylabel(parm.ilabel, fontsize=parm.fontsize_label)

    ax1.xaxis.set_major_locator(mpl.ticker.MultipleLocator(0.75))
    ax1.yaxis.set_major_locator(mpl.ticker.MaxNLocator(7))

    formatter = mpl.ticker.FormatStrFormatter(mtext('%.2f'))
    ax1.xaxis.set_major_formatter(formatter)
    ax1.yaxis.set_major_formatter(formatter)
    plt.tick_params(labelsize=parm.fontsize_ticks)

    oname = 'iv%s.pdf' % opts.stub
    plt.savefig(oname)
    print 'saved: %s' % oname

def plotJV():
    parm = PlotParams(xmin=-0.75, xmax=3.0)

    fig, ax1 = langmuir.plot.subplots(**parm.kws_subplots)

    langmuir.plot.rectangle(0, 0, iv.v_oc, iv.d_sc, **parm.kws_orect)
    langmuir.plot.rectangle(0, 0, iv.v_mp, iv.d_mp, **parm.kws_irect)

    plt.axhline(iv.d_sc, **parm.kws_span)
    plt.axhline(iv.d_mp, **parm.kws_span)
    plt.axvline(iv.v_oc, **parm.kws_span)
    plt.axvline(iv.v_mp, **parm.kws_span)

    plt.axhline(0, **parm.kws_axes)
    plt.axvline(0, **parm.kws_axes)

    plt.errorbar(iv.v, iv.d, yerr=iv.derr, **parm.kws_idata)
    plt.errorbar(iv.v, iv.r, yerr=iv.rerr, **parm.kws_pdata)

    iv.dfit.plot(parm.xmin, parm.xmax, **parm.kws_ifit)
    iv.rfit.plot(parm.xmin, parm.xmax, **parm.kws_pfit)

    ltable(parm, iv, density=True, pfmt='%.0f', ifmt='%.0f')
    plt.legend(**parm.kws_legend)

    plt.xlim(parm.xmin, parm.xmax)

    plt.ylim(iv.dstats.min, iv.dstats.max)
    langmuir.plot.zoom(l=0, r=0, factor=0.25)
    symmetric_ylimits()

    plt.title(parm.title, fontsize=parm.fontsize_title)
    plt.xlabel(parm.vlabel, fontsize=parm.fontsize_label)
    plt.ylabel(parm.dlabel, fontsize=parm.fontsize_label)

    ax1.xaxis.set_major_locator(mpl.ticker.MultipleLocator(0.75))
    ax1.yaxis.set_major_locator(mpl.ticker.MaxNLocator(7))

    formatter = mpl.ticker.FormatStrFormatter(mtext('%.2f'))
    ax1.xaxis.set_major_formatter(formatter)

    formatter = mpl.ticker.FormatStrFormatter(mtext('%.0f'))
    ax1.yaxis.set_major_formatter(formatter)
    plt.tick_params(labelsize=parm.fontsize_ticks)

    oname = 'jv%s.pdf' % opts.stub
    plt.savefig(oname)
    print 'saved: %s' % oname

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    iv = langmuir.ivline.IVLineS()
    iv.load_pkls(opts.pkls)

    iv.calculate(s=opts.shift, mode=opts.mode, recycle=opts.recycle,
        order=opts.order, kind=opts.kind)

    oname = 'fit%s.csv' % opts.stub
    iv.csv(oname)
    print 'saved: %s' % oname

    oname = 'fit%s.pkl' % opts.stub
    iv.pkl(oname)
    print 'saved: %s' % oname

    if opts.plot:
        print '...'
        plotIV()
        plotJV()

        if opts.show:
            plt.show()
