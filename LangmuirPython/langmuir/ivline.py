# -*- coding: utf-8 -*-
from langmuir.plot import mtext
import collections as _collections
import matplotlib.pyplot as _plt
import matplotlib as _mpl
import pickle as _pickle
import StringIO as _StringIO
import pandas as _pd
import numpy as _np
import sys as _sys
import langmuir

#_mpl.rc('mathtext', default='tt', fontset='cm')
#_mpl.rc('text', usetex=True)

class units(object):
    """
    Struct to manage units for IV curves.

    ===== ===============
    Data  Description
    ===== ===============
    **v** voltage
    **i** current
    **p** power
    **d** current density
    **r** power density
    **a** area
    ===== ===============

    ============ ============================
    Attribute    Description
    ============ ============================
    ?            v, i, p, d, r
    ?units_input units assumed for input data
    ?units       units used for quantity
    ?str1        units as latex str
    ?str2        units as latex str (no frac)
    q            quantities module
    ============ ============================
    """
    try:
        import quantities as q
        q.nW = q.UnitQuantity('nanowatt', q.nano * q.W, 'nW')

        vunits_input = q.V
        iunits_input = q.nA
        aunits_input = q.nm**2

        vunits = q.V
        iunits = q.nA
        punits = q.nW
        dunits = q.mA / q.cm**2
        runits = q.mW / q.cm**2
        aunits = q.nm**2
    except:
        print >> _sys.stderr, 'langmuir: can not import quantities module'
        print >> _sys.stderr, 'langmuir: ivline module can not be used'

    vstr1 = str(vunits.dimensionality)
    istr1 = str(iunits.dimensionality)
    pstr1 = str(punits.dimensionality)
    dstr1 = str(dunits.dimensionality)
    rstr1 = str(runits.dimensionality)
    astr1 = str(aunits.dimensionality)

    vstr2 = r'V'
    istr2 = r'nA'
    pstr2 = r'nW'
    dstr2 = r'mA\,cm^{-2}'
    rstr2 = r'mW\,cm^{-2}'
    astr2 = r'nm^{2}'

    @staticmethod
    def to_units(u):
        """
        Turn object into :class:`quantities.Quantity` and extract units object

        :param u: object with units, can be a :py:obj`str`.
        """
        if isinstance(u, str):
            u = units.q.CompoundUnit(u)
        u = units.q.Quantity(u)
        return u.units

    @staticmethod
    def cfactor(u1, u2):
        """
        Compute conversion factor.

        :param u1: object with units, can be a :py:obj`str`.
        :param u2: object with units, can be a :py:obj`str`.

        :returns: float factor
        """
        u1 = units.to_units(u1)
        u2 = units.to_units(u2)
        return float(u1.rescale(u2))

    @staticmethod
    def rescale(obj, u1, u2=None):
        """
        Rescale units of object.

        :param obj: object with or without units
        :param u1: object with units, can be a :py:obj`str`.
        :param u2: object with units, can be a :py:obj`str` or :py:obj:`None`.

        >>> obj = units.rescale([1, 2, 3], 'nA', 'A')        # nA   -> A
        >>> obj = units.rescale([1, 2, 3], 'A')              # None -> A
        >>> obj = units.rescale([1] * units.q.nA, 'A')       # nA   -> A
        >>> obj = units.rescale([1] * units.q.nA, 'A', 'nA') # nA   -> A -> nA

        returns: :py:class:`quantities.Quantity`
        """
        u1 = units.to_units(u1)
        if u2 is None:
            if hasattr(obj, 'rescale'):
                return obj.rescale(u1)
            return _np.array(obj) * u1
        if hasattr(obj, 'rescale'):
            return obj.rescale(u1).rescale(u2)
        return (_np.array(obj) * u1).rescale(u2)

    @staticmethod
    def unitDict():
        d = _collections.OrderedDict()
        d['vunits'] = units.vstr1
        d['iunits'] = units.istr1
        d['punits'] = units.pstr1
        d['dunits'] = units.dstr1
        d['runits'] = units.rstr1
        d['dunits'] = units.astr1
        return d

class Stats(object):
    """
    Compute various statistics of an array like object.

    ======== ==========================
    Attr     Description
    ======== ==========================
    **name** stub
    **max**  max of data
    **min**  min of data
    **rng**  range of data
    **avg**  average of data
    **std**  standard deviation of data
    ======== ==========================

    >>> s = Stats([1, 2, 3, 4, 5])
    """
    def __init__(self, array, name):
        self.name = name
        self.max = _np.amax(array)
        self.min = _np.amin(array)
        self.rng = abs(self.max - self.min)
        self.avg = _np.average(array)
        self.std = _np.std(array)
        try:
            self.units = array.units.dimensionality
        except:
            self.units = ''

    def dataDict(self):
        """
        Get summary of stats.
        """
        d = _collections.OrderedDict()
        d['%smax' % self.name] = float(self.max)
        d['%smin' % self.name] = float(self.min)
        d['%srng' % self.name] = float(self.rng)
        d['%savg' % self.name] = float(self.max)
        d['%sstd' % self.name] = float(self.max)
        return d

    def __str__(self):
        s = _StringIO.StringIO()
        print >> s, r'%smax = %+.5e %s' % (self.name, self.max, self.units)
        print >> s, r'%smin = %+.5e %s' % (self.name, self.min, self.units)
        print >> s, r'%srng = %+.5e %s' % (self.name, self.rng, self.units)
        print >> s, r'%savg = %+.5e %s' % (self.name, self.avg, self.units)
        print >> s, r'%sstd = %+.5e %s' % (self.name, self.std, self.units)
        return s.getvalue()

class IVLine(object):
    """
    Collection of data for an IV curve and methods to analyze it

    >>> iv = IVLine()
    >>> iv.load_pkls(['run_0.pkl', 'run_1.pkl'])
    >>> iv.calculate()
    """
    def __init__(self, *args, **kwargs):
        pass

    @classmethod
    def from_search(cls, path, **kwargs):
        """
        Alternative constructor.  Search the path for a set of pkl files,
        and load them into an IVLine instance.

        :param path: path to search
        :type path: str

        :returns: :py:class:`IVLine`
        """
        _kwargs = dict(stub='gathered*', recursive=True, at_least_one=True)
        _kwargs.update(**kwargs)
        paths = langmuir.find.pkls(path, **_kwargs)
        ivline = cls()
        ivline.load_pkls(paths)
        ivline.calculate()
        return ivline

    @classmethod
    def from_pkls(cls, pkls):
        """
        Alternative constructor.  Creates instance and loads the pkl files.

        :param pkls: list of paths
        :type pkls: str

        :returns: :py:class:`IVLine`
        """
        ivline = cls()
        ivline.load_pkls(pkls)
        return ivline

    def load_pkls(self, paths, **kwargs):
        """
        Load a set of pkl files created using langmuir.analyze into IVLine.

        :param paths: paths to pkl files
        :type paths: list

        >>> iv.load_pkls(['run_0.pkl', 'run_1.pkl'])
        >>> iv.calculate()
        """
        panel = langmuir.analyze.load_pkls(paths, **kwargs)
        self.process_panel(panel)

    def process_panel(self, panel=None):
        """
        Extract current from panda's panel.
        """
        if panel:
            self.panel = panel

        self.i = panel.minor_xs('drain:current').mean(axis=1)
        self.i = units.rescale(self.i, units.iunits_input, units.iunits)

        self.v = panel.minor_xs('voltage.right').mean(axis=1)
        self.v = units.rescale(self.v, units.vunits_input, units.vunits)

        grid_x = _np.array(panel.minor_xs('grid.x').mean(axis=1))
        grid_y = _np.array(panel.minor_xs('grid.x').mean(axis=1))
        self.a = _np.average(grid_x * grid_y) * units.aunits
        self.a = units.rescale(self.a, units.aunits_input, units.aunits)
        self.a = float(_np.average(self.a)) * units.aunits

        self.ierr = panel.minor_xs('drain:current').std(axis=1).fillna(0)
        self.ierr = units.rescale(self.ierr, units.iunits_input, units.iunits)

    def calculate(self, panel=None):
        """
        Calculate power, density, etc from loaded data.

        >>> iv.calculate()
        """
        if panel:
            self.process_panel(panel)

        self.p = units.rescale(self.i * self.v, units.punits)
        self.d = units.rescale(self.i / self.a, units.dunits)
        self.r = units.rescale(self.p / self.a, units.runits)

        self.perr = units.rescale(self.ierr * self.v, units.punits)
        self.derr = units.rescale(self.ierr / self.a, units.dunits)
        self.rerr = units.rescale(self.perr / self.a, units.runits)

        self.vstats = Stats(self.v, 'v')
        self.istats = Stats(self.i, 'i')
        self.pstats = Stats(self.p, 'p')
        self.dstats = Stats(self.d, 'd')
        self.rstats = Stats(self.r, 'r')

    def data(self):
        """
        Convert data into a python dict
        """
        d = _collections.OrderedDict()
        d['v_avg'] = _np.array(self.v)
        d['i_avg'] = _np.array(self.i)
        d['i_std'] = _np.array(self.ierr)
        d['a_avg'] = _np.ones(self.i.size) * float(self.a)
        d['p_avg'] = _np.array(self.p)
        d['p_std'] = _np.array(self.perr)
        d['d_avg'] = _np.array(self.d)
        d['d_std'] = _np.array(self.derr)
        d['r_avg'] = _np.array(self.r)
        d['r_std'] = _np.array(self.rerr)
        return d

    def results(self):
        """
        Convert calculate results to a python dict
        """
        return {}

    def dataframe(self):
        """
        Convert data into a pandas DataFrame
        """
        d = self.data()
        return _pd.DataFrame(dict(d), columns=d.keys())

    def csv(self, handle):
        """
        Save data to a CSV file.
        """
        if isinstance(handle, str):
            handle = langmuir.common.zhandle(handle, 'w')
        df = self.dataframe()
        df.to_csv(handle, index=False)
        handle.close()

    def pkl(self, handle):
        """
        Save data to a CSV file.
        """
        if isinstance(handle, str):
            handle = langmuir.common.zhandle(handle, 'w')
        results = {}
        results.update(self.data())
        results.update(self.results())
        _pickle.dump(results, handle, _pickle.HIGHEST_PROTOCOL)
        return handle

class IVLineS(IVLine):
    """
    An IVLine specialized for solar cells.
    """
    def __init__(self, *args, **kwargs):
        IVLine.__init__(self, *args, **kwargs)

    def test(self, points=20, error=0.05):
        """
        Create some test data.

        :param points: number of points
        :param error: std of simulated error

        :type points: int
        :type error: float

        >>> iv.test()
        >>> iv.calculate()
        """
        self.v = _np.linspace(-5, 5, points)
        self.i = _np.tanh(0.5 * self.v - 0.5) + \
            _np.random.normal(0, error, self.v.size)
        self.a = 256.0 ** 2
        self.ierr = _np.random.normal(0, error, self.i.size)
        self.v = units.rescale(self.v, units.vunits_input, units.vunits)
        self.i = units.rescale(self.i, units.iunits_input, units.iunits)
        self.a = units.rescale(self.a, units.aunits_input, units.aunits)
        self.ierr = units.rescale(self.ierr, units.iunits_input, units.iunits)
        self.calculate()

    def calculate(self, s=1.5, mode='tanh', recycle=False, order=8, k=2,
                  kind='linear'):
        """
        Calculate fill factor.

        :param s: voltage shift
        :param mode: fitting mode (tanh, power, interp1d)
        :param order: order for power fit
        :param kind: kind for interp1d
        :param recycle: reuse popt from current fit for power fit
        :param k: order of univariate spline

        :type s: float
        :type mode: str
        :type order: int
        :type kind: str
        :type recycle: bool
        :type k: int

        >>> iv.calculate(langmuir.fit.FitTanh, langmuir.fit.FitXTanh)
        """
        self.s = float(s) * units.vunits
        self.v = self.v + self.s
        IVLine.calculate(self)

        if mode == 'power':
            self.ifit = langmuir.fit.FitPower(self.v, self.i, order=order,
                                              popt=None, yerr=self.ierr)

            self.dfit = langmuir.fit.FitPower(self.v, self.d, order=order,
                                              popt=None, yerr=self.derr)

            if recycle:
                p_popt = self.ifit.popt
                r_popt = self.dfit.popt
            else:
                p_popt = None
                r_popt = None

            self.pfit = langmuir.fit.FitPower(self.v, self.p, order=order + 1,
                                              popt=p_popt, yerr=self.perr)

            self.rfit = langmuir.fit.FitPower(self.v, self.r, order=order + 1,
                                              popt=r_popt, yerr=self.rerr)

        elif mode == 'tanh':
            popt = [self.istats.min, 1, -s, 0]
            self.ifit = langmuir.fit.FitTanh(self.v, self.i, popt=popt,
                                             yerr=self.ierr)

            popt = [self.dstats.min, 1, -s, 0]
            self.dfit = langmuir.fit.FitTanh(self.v, self.d, popt=popt,
                                              yerr=self.derr)

            if recycle:
                p_popt = self.ifit.popt
                r_popt = self.dfit.popt
            else:
                p_popt = popt
                r_popt = popt

            self.pfit = langmuir.fit.FitXTanh(self.v, self.p, popt=p_popt,
                                              yerr=self.perr)

            self.rfit = langmuir.fit.FitXTanh(self.v, self.r, popt=r_popt,
                                              yerr=self.rerr)

        elif mode == 'erf':
            popt = [self.istats.min, 1, -s, 0]
            self.ifit = langmuir.fit.FitErf(self.v, self.i, popt=popt,
                                             yerr=self.ierr)

            popt = [self.dstats.min, 1, -s, 0]
            self.dfit = langmuir.fit.FitErf(self.v, self.d, popt=popt,
                                              yerr=self.derr)

            if recycle:
                p_popt = self.ifit.popt
                r_popt = self.dfit.popt
            else:
                p_popt = popt
                r_popt = popt

            self.pfit = langmuir.fit.FitXErf(self.v, self.p, popt=p_popt,
                                             yerr=self.perr)

            self.rfit = langmuir.fit.FitXErf(self.v, self.r, popt=r_popt,
                                             yerr=self.rerr)

        elif mode == 'interp1d':
            self.ifit = langmuir.fit.FitInterp1D(self.v, self.i, popt=None,
                                                 yerr=self.ierr, kind=kind)

            self.dfit = langmuir.fit.FitInterp1D(self.v, self.d, popt=None,
                                                 yerr=self.derr, kind=kind)

            self.pfit = langmuir.fit.FitInterp1D(self.v, self.p, popt=None,
                                                 yerr=self.perr, kind=kind)

            self.rfit = langmuir.fit.FitInterp1D(self.v, self.r, popt=None,
                                                 yerr=self.rerr, kind=kind)


        elif mode == 'spline':
            self.ifit = langmuir.fit.FitUnivariateSpline(self.v, self.i,
                                        popt=None, yerr=self.ierr, k=k)

            self.dfit = langmuir.fit.FitUnivariateSpline(self.v, self.d,
                                        popt=None, yerr=self.derr, k=k)

            self.pfit = langmuir.fit.FitUnivariateSpline(self.v, self.p,
                                        popt=None, yerr=self.perr, k=k)

            self.rfit = langmuir.fit.FitUnivariateSpline(self.v, self.r,
                                        popt=None, yerr=self.rerr, k=k)


        else:
            raise NotImplementedError('unknown fit mode: %s' % mode)

        self.v_oc = self.ifit.solve(x0=self.s) * units.vunits
        self.i_sc = self.ifit(0) * units.iunits
        self.p_th = units.rescale(self.v_oc * self.i_sc, units.punits)

        self.v_mp = self.pfit.minimize(0.66 * self.v_oc, return_y=False) * units.vunits
        self.i_mp = self.ifit(float(self.v_mp)) * units.iunits
        self.p_mp = units.rescale(self.i_mp * self.v_mp, units.punits)

        self.d_sc = units.rescale(self.i_sc / self.a, units.dunits)
        self.d_mp = units.rescale(self.i_mp / self.a, units.dunits)

        self.r_th = units.rescale(self.p_th / self.a, units.runits)
        self.r_mp = units.rescale(self.p_mp / self.a, units.runits)

        self.fill = self.p_mp / self.p_th * 100 * units.q.percent

    def results(self):
        """
        Get a summary of results.
        """
        d = _collections.OrderedDict()
        d['v_oc'] = (float(self.v_oc), units.vstr1, units.vstr2, r'V_{oc}')
        d['v_mp'] = (float(self.v_mp), units.vstr1, units.vstr2, r'V_{mp}')
        d['i_sc'] = (float(self.i_sc), units.istr1, units.istr2, r'I_{sc}')
        d['i_mp'] = (float(self.i_mp), units.istr1, units.istr2, r'I_{mp}')
        d['p_th'] = (float(self.p_th), units.pstr1, units.pstr2, r'P_{th}')
        d['p_mp'] = (float(self.p_mp), units.pstr1, units.pstr2, r'P_{mp}')
        d['d_sc'] = (float(self.d_sc), units.dstr1, units.dstr2, r'J_{sc}')
        d['d_mp'] = (float(self.d_mp), units.dstr1, units.dstr2, r'J_{mp}')
        d['r_th'] = (float(self.r_th), units.rstr1, units.rstr2, r'R_{th}')
        d['r_mp'] = (float(self.r_mp), units.rstr1, units.rstr2, r'R_{mp}')
        d['fill'] = (float(self.fill), r'%', r'\%', r'FF')
        d['s'] = (float(self.s), units.vstr1, r'V_{s}')
        return d

    def __str__(self):
        s = _StringIO.StringIO()
        e = '+12.5e'
        print >> s, 'v_oc = {v_oc[0]:{e}} {v_oc[1]}'
        print >> s, 'v_mp = {v_mp[0]:{e}} {v_mp[1]}'
        print >> s, 'i_sc = {i_sc[0]:{e}} {i_sc[1]} {d_sc[0]:{e}} {d_sc[1]}'
        print >> s, 'i_mp = {i_mp[0]:{e}} {i_mp[1]} {d_mp[0]:{e}} {d_mp[1]}'
        print >> s, 'p_th = {p_th[0]:{e}} {p_th[1]} {r_th[0]:{e}} {r_th[1]}'
        print >> s, 'p_mp = {p_mp[0]:{e}} {p_mp[1]} {r_mp[0]:{e}} {r_mp[1]}'
        print >> s, 'fill = {fill[0]:{e}} {fill[1]}'
        return s.getvalue().format(e=e, **self.results())

class IVLineT(IVLine):
    def __init__(self, *args, **kwargs):
        IVLine.__init__(self, *args, **kwargs)
        self.fits = []

    def fit(self, s=None):
        if s:
            x, y, yerr = self.v[s], self.i[s], self.ierr[s]
        else:
            x, y, yerr = self.v, self.i, self.ierr
        self.fits.append(langmuir.fit.FitLinear(x, y, yerr=yerr))
        return self.fits[-1]

    def results(self):
        """
        Get a summary of results.
        """
        d = _collections.OrderedDict()
        d['fits'] = [f.summary() for f in self.fits]
        return d

class IVPlot(object):
    def __init__(self, xmin=None, xmax=None, ymin=None, ymax=None,
                 **kwargs):
        self.fig, self.ax1 = langmuir.plot.subplots(**kwargs)
        self.vlabel = mtext(r'V\,(%s)' % units.vstr2)
        self.ilabel = mtext(r'I\,(%s)' % units.istr2)
        self.plabel = mtext(r'P\,(%s)' % units.pstr2)
        self.dlabel = mtext(r'J\,(%s)' % units.dstr2)
        self.rlabel = mtext(r'R\,(%s)' % units.rstr2)
        self.xmin, self.xmax = xmin, xmax
        self.ymin, self.ymax = ymin, ymax

    def axhline(self, y, color='k', lw=1, ls=':', zorder=-1000, text='',
                xtext=0.1, **kwargs):
        _plt.axhline(y, color=color, lw=lw, ls=ls, zorder=zorder)
        if text:
            xtrans = _plt.gca().transAxes
            ytrans = _plt.gca().transData
            ttrans = _mpl.transforms.blended_transform_factory(xtrans, ytrans)
            _kwargs = dict(transform=ttrans, ha='center', va='center',
                           bbox=dict(fc='w', lw=0, pad=10))
            _kwargs.update(**kwargs)
            _plt.text(xtext, y, text, **_kwargs)

    def axvline(self, x, color='k', lw=1, ls=':', zorder=-1000, text='',
                ytext=0.1, **kwargs):
        _plt.axvline(x, color=color, lw=lw, ls=ls, zorder=zorder)
        if text:
            ytrans = _plt.gca().transAxes
            xtrans = _plt.gca().transData
            ttrans = _mpl.transforms.blended_transform_factory(xtrans, ytrans)
            _kwargs = dict(transform=ttrans, ha='center', va='center',
                           bbox=dict(fc='w', lw=0, pad=10), rotation=90)
            _kwargs.update(**kwargs)
            _plt.text(x, ytext, text, **_kwargs)

    def ax_text(self, x, y, s, **kwargs):
        _kwargs = dict(transform=self.ax1.transAxes, ha='center', va='center',
                       bbox=dict(fc='w', lw=0, pad=10))
        _kwargs.update(**kwargs)
        return _plt.text(x, y, s, **kwargs)

    def errorbar(self, x, y, yerr=None, color='r', marker='o', **kwargs):
        _kwargs = dict(yerr=yerr, color=color, marker=marker, ls='',
                       mec=color, mfc='w', capsize=5)
        _kwargs.update(kwargs)
        _plt.errorbar(x, y, **_kwargs)

    def rectangle(self, v0, i0, v1, i1, axis=None, **kwargs):
        v0, i0, v1, i1 = float(v0), float(i0), float(v1), float(i1)
        _kwargs = dict(fc='r', lw=0, alpha=0.5)
        _kwargs.update(kwargs)
        rect = _mpl.patches.Rectangle((v0, i0), v1 - v0, i1 - i0, **_kwargs)
        _plt.gca().add_patch(rect)
        return rect

    def plot_axes(self, **kwargs):
        _kwargs = dict(color='k', lw=1, zorder=+1000)
        _kwargs.update(kwargs)
        _plt.axhline(0, **_kwargs)
        _plt.axvline(0, **_kwargs)

    def title(self, title, **kwargs):
        _kwargs = dict(fontsize='xx-large', ha='center', va='bottom',
                       transform=self.ax1.transAxes)
        _kwargs.update(**kwargs)
        if title:
            _plt.text(0.5, 1.025, title, **_kwargs)

    def legend(self, zorder=None, **kwargs):
        _kwargs = dict(loc='best', borderaxespad=1, frameon=False,
                       prop=dict(size='medium'))
        _kwargs.update(**kwargs)
        self._legend = _plt.legend(**_kwargs)
        if not zorder is None:
            self._legend.zorder=zorder

    def setFormatter(self, formatter, x=True, y=True, major=True, minor=False):
        if major:
            if x: self.ax1.xaxis.set_major_formatter(formatter)
            if y: self.ax1.yaxis.set_major_formatter(formatter)
        if minor:
            if x: self.ax1.xaxis.set_minor_formatter(formatter)
            if y: self.ax1.yaxis.set_minor_formatter(formatter)

    def setLocator(self, locator, x=True, y=True, major=True, minor=False):
        if major:
            if x: self.ax1.xaxis.set_major_locator(locator)
            if y: self.ax1.yaxis.set_major_locator(locator)
        if minor:
            if x: self.ax1.xaxis.set_minor_locator(locator)
            if y: self.ax1.yaxis.set_minor_locator(locator)

    def finish(self, ylabel=None, xlabel=None):
        _plt.sca(self.ax1)
        if ylabel is None: ylabel = self.ilabel
        if xlabel is None: xlabel = self.vlabel
        _plt.xlabel(xlabel, fontsize='x-large')
        _plt.ylabel(ylabel, fontsize='x-large')
        _plt.tick_params(labelsize='x-large', direction='in')
        _plt.locator_params(nbins=6)
        _plt.xlim(self.xmin, self.xmax)
        _plt.ylim(self.ymin, self.ymax)

    def show(self):
        self.finish()
        _plt.show()

class IVPlotS(IVPlot):
    def __init__(self, xmin=None, xmax=None, ymin=None, ymax=None,
                 **kwargs):
        IVPlot.__init__(self, xmin, xmax, ymin, ymax, **kwargs)

class IVPlotT(IVPlot):
    def __init__(self, xmin=None, xmax=None, ymin=None, ymax=None,
                 **kwargs):
        IVPlot.__init__(self, xmin, xmax, ymin, ymax, **kwargs)
        self.vlabel = mtext(r'V_{DS}\,(%s)' % units.vstr2)
        self.ilabel = mtext(r'I_{DS}\,(%s)' % units.istr2)
