# -*- coding: utf-8 -*-
"""
.. note::
    Functions for analyzing current voltage data.

.. moduleauthor:: Adam Gagorik <adam.gagorik@gmail.com>
"""
from langmuir import units, Quantity
import langmuir as lm
import pandas as pd
import numpy as np
import collections
import StringIO

class IVCurve(object):
    """
    A class to analyze IV curves.

    ======== ==========================
    Attr     Description
    ======== ==========================
    **i**    current
    **v**    voltage
    **p**    power
    **a**    area
    **j**    current density
    **r**    irradiance
    ======== ==========================

    .. seealso::
        :py:class:`IVCurveSolar`
    """
    class units:
        def __init__(self):
            pass

        try:
            i = units.nA
            v = units.V
            p = units.nW
            a = units.nm**2
            j = units.mA/units.cm**2
            r = units.mW/units.cm**2
        except:
            i = None
            v = None
            p = None
            a = None
            j = None
            r = None

    class latex:
        def __init__(self):
            pass

        i = r'\ensuremath{nA}'
        v = r'\ensuremath{V}'
        p = r'\ensuremath{nW}'
        a = r'\ensuremath{nm^{2}}'
        j = r'\ensuremath{mA\,cm^{-2}}'
        r = r'\ensuremath{mW\,cm^{-2}}'

    class string:
        def __init__(self):
            pass

        i = r'nA'
        v = r'V'
        p = r'nW'
        a = r'nm**2'
        j = r'mA/cm**2'
        r = r'mW/cm**2'

    class columns:
        def __init__(self):
            pass

        i = r'drain:current'
        v = r'voltage.right'
        x = r'grid.x'
        y = r'grid.y'

    def __init__(self, i, v, a, ierr=None):
        """
        Construct IV curve.

        :param i: current
        :param v: voltage
        :param a: area
        :param ierr: std of current

        :type i: :py:class:`numpy.ndarray`
        :type v: :py:class:`numpy.ndarray`
        :type a: :py:class:`numpy.ndarray` or :py:class:`float`
        :type e: :py:class:`numpy.ndarray` or None

        >>> v = np.linspace(0, 100, 10)
        >>> i = np.tanh(v)
        >>> a = 1024 * 256
        >>> ivcurve = lm.ivcurve.IVCurve(i, v, a)

        .. seealso::
            :py:meth:`IVCurve.from_dataframe`
            :py:meth:`IVCurve.from_panel`
            :py:meth:`IVCurve.load_pkl`
            :py:meth:`IVCurve.load_pkls`
        """
        i = np.asanyarray(i)
        v = np.asanyarray(v)

        if np.isscalar(a):
            a = np.ones_like(i) * a
        else:
            a = np.asanyarray(a)

        assert i.shape == v.shape == a.shape

        ### <<< units on
        self.i = Quantity(i, self.units.i)          # current
        self.v = Quantity(v, self.units.v)          # voltage
        self.a = Quantity(a, self.units.a)          # area

        self.p = (self.i * self.v).to(self.units.p) # power
        self.j = (self.i / self.a).to(self.units.j) # density
        self.r = (self.p / self.a).to(self.units.r) # irradiance

        if ierr is None:
            ierr = np.zeros_like(i)
        else:
            ierr = np.asanyarray(ierr)

        self.ierr = Quantity(ierr, self.units.i)
        self.perr = (self.ierr * self.v).to(self.units.p) # power
        self.jerr = (self.ierr / self.a).to(self.units.j) # density
        self.rerr = (self.perr / self.a).to(self.units.r) # irradiance

        ### <<< units off
        self.i = np.asanyarray(self.i.magnitude)
        self.v = np.asanyarray(self.v.magnitude)
        self.p = np.asanyarray(self.p.magnitude)
        self.a = np.asanyarray(self.a.magnitude)
        self.j = np.asanyarray(self.j.magnitude)
        self.r = np.asanyarray(self.r.magnitude)

        self.ierr = np.asanyarray(self.ierr.magnitude)
        self.perr = np.asanyarray(self.perr.magnitude)
        self.jerr = np.asanyarray(self.jerr.magnitude)
        self.rerr = np.asanyarray(self.rerr.magnitude)

        class stats:
            def __init__(self):
                pass

            i = lm.analyze.Stats(self.i, 'i')
            v = lm.analyze.Stats(self.v, 'v')
            p = lm.analyze.Stats(self.p, 'p')
            a = lm.analyze.Stats(self.a, 'a')
            j = lm.analyze.Stats(self.j, 'j')
            r = lm.analyze.Stats(self.r, 'r')
            ierr = lm.analyze.Stats(self.ierr, 'ierr')
            jerr = lm.analyze.Stats(self.jerr, 'jerr')
            perr = lm.analyze.Stats(self.perr, 'perr')
            rerr = lm.analyze.Stats(self.rerr, 'rerr')

        self.stats = stats

    @classmethod
    def from_dataframe(cls, frame):
        """
        Construct IV curve from pandas DataFrame.

        :param panel: dataframe object
        :type panel: :py:class:`pandas.DataFrame`

        >>> frame = lm.common.load_pkl('calculated.pkl.gz')
        >>> ivcurve = lm.ivcurve.IVCurve(frame)

        .. seealso::
            :py:meth:`IVCurve.load_pkl`
        """
        i = np.asanyarray(frame[cls.columns.i])
        v = np.asanyarray(frame[cls.columns.v])

        x = np.asanyarray(frame[cls.columns.x])
        y = np.asanyarray(frame[cls.columns.y])
        a = x * y

        IVCurve = cls(i, v, a)
        return IVCurve

    @classmethod
    def from_panel(cls, panel):
        """
        Construct IV curve from pandas Panel.

        :param panel: panel object
        :type panel: :py:class:`pandas.Panel`

        >>> pkls = lm.find.pkls('.', stub='gathered*', r=True)
        >>> panel = lm.analyze.create_panel(lm.common.load_pkls(pkls))
        >>> ivcurve = lm.ivcurve.IVCurve.from_panel(panel)

        .. seealso::
            :py:meth:`IVCurve.load_pkls`
        """
        i = np.asanyarray(
            panel.minor_xs(cls.columns.i).mean(axis=1).fillna(0))
        v = np.asanyarray(
            panel.minor_xs(cls.columns.v).mean(axis=1).fillna(0))

        grid_x = np.asanyarray(panel.minor_xs(cls.columns.x).mean(axis=1))
        grid_y = np.asanyarray(panel.minor_xs(cls.columns.y).mean(axis=1))
        a = grid_x * grid_y

        ierr = np.asanyarray(
            panel.minor_xs(cls.columns.i).std(axis=1).fillna(0))

        IVCurve = cls(i, v, a, ierr)
        return IVCurve

    @classmethod
    def load_pkl(cls, pkl):
        """
        Construct IV curve from pkl file.

        :param pkl: name of file
        :type pkl: str

        >>> ivcurve = lm.ivcurve.IVCurve.load_pkl('gathered.pkl.gz')
        """
        pkl = lm.common.load_pkl(pkl)
        return cls.from_dataframe(pkl)

    @classmethod
    def load_pkls(cls, pkls):
        """
        Construct IV curve from a list of pkl files.

        :param pkls: list of filenames.
        :type pkls: list

        >>> pkls = lm.find.pkls('.', stub='gathered*', r=True)
        >>> ivcurve = lm.ivcurve.IVCurve.load_pkls(pkls)
        """
        panel = lm.analyze.create_panel(lm.common.load_pkls(pkls))
        return cls.from_panel(panel)

    def to_dict(self):
        """
        Get data as dict.
        """
        d = collections.OrderedDict()
        d['v'   ] = self.v
        d['i'   ] = self.i
        d['ierr'] = self.ierr
        d['p'   ] = self.p
        d['perr'] = self.perr
        d['a'   ] = self.a
        d['j'   ] = self.j
        d['jerr'] = self.jerr
        d['r'   ] = self.r
        d['rerr'] = self.rerr
        return d

    def to_dataframe(self):
        """
        Convert data into a :py:class:`pandas.DataFrame`
        """
        d = self.to_dict()
        return pd.DataFrame(dict(d), columns=d.keys())

    def save_csv(self, handle, **kwargs):
        """
        Save data to a CSV file.

        :param handle: filename
        :type handle: str
        """
        handle = lm.common.zhandle(handle, 'wb')
        frame = self.to_dataframe()
        _kwargs = dict(index=True)
        _kwargs.update(**kwargs)
        frame.to_csv(handle, **_kwargs)
        handle.write('#\n')
        series = self.to_series()
        series.to_csv(handle, **_kwargs)

    def summary(self):
        """
        Get summary of all calculated results as a dictionary.
        """
        d = collections.OrderedDict()
        d.update(self.stats.i.to_dict())
        d.update(self.stats.v.to_dict())
        d.update(self.stats.p.to_dict())
        d.update(self.stats.a.to_dict())
        d.update(self.stats.j.to_dict())
        d.update(self.stats.r.to_dict())
        d.update(self.stats.ierr.to_dict())
        d.update(self.stats.jerr.to_dict())
        d.update(self.stats.perr.to_dict())
        d.update(self.stats.rerr.to_dict())
        return d

    def to_series(self):
        """
        Convert data into a :py:class:`pandas.Series`
        """
        d = self.summary()
        return pd.Series(d, index=d.keys())

    def save_pkl(self, handle):
        """
        Save results and data to a PKL file.

        :param handle: filename
        :type handle: str
        """
        results = self.to_dict()
        results.update(self.summary())
        lm.common.save_pkl(results, handle)

class IVCurveSolar(IVCurve):
    """
    A class to analyze IV curves for solar cells.  Calculates the fill factor.

    ======== ==========================
    Attr     Description
    ======== ==========================
    **v_oc** open circuit voltage
    **v_mp** voltage at max power
    **i_sc** short circuit current
    **i_mp** current at max power
    **p_th** theoretical power
    **p_mp** max power
    **fill** fill factor
    ======== ==========================
    """

    class latex(IVCurve.latex):
        def __init__(self):
            pass

        f    = r'\%'
        v_oc = r'\ensuremath{v_{oc}}'
        i_sc = r'\ensuremath{i_{sc}}'
        p_th = r'\ensuremath{p_{th}}'
        j_sc = r'\ensuremath{j_{sc}}'
        r_th = r'\ensuremath{r_{th}}'
        i_mp = r'\ensuremath{i_{mp}}'
        v_mp = r'\ensuremath{v_{mp}}'
        p_mp = r'\ensuremath{p_{mp}}'
        j_mp = r'\ensuremath{j_{mp}}'
        r_mp = r'\ensuremath{r_{mp}}'
        fill = r'\ensuremath{FF}'

    class string(IVCurve.string):
        def __init__(self):
            pass

        f    = r'%'
        v_oc = r'v_oc'
        i_sc = r'i_sc'
        p_th = r'p_th'
        j_sc = r'j_sc'
        r_th = r'r_th'
        i_mp = r'i_mp'
        v_mp = r'v_mp'
        p_mp = r'p_mp'
        j_mp = r'j_mp'
        r_mp = r'r_mp'
        fill = r'fill'

    def __init__(self, i, v, a, ierr=None, v_shift=1.5, **kwargs):
        """
        Construct IV curve.  See arguments for :py:class:`IVCurve`.

        :param v_shift: amount to shift voltage axis by
        :type v_shift: float

        >>> v = np.linspace(0, 100, 10)
        >>> i = np.tanh(v)
        >>> a = 1024 * 256
        >>> ivcurve = lm.ivcurve.IVCurveSolar(i, v, a)
        """
        self.v_shift = v_shift
        v = np.asanyarray(v) + v_shift
        super(IVCurveSolar, self).__init__(i, v, a, ierr)
        self.v_oc = 0.0
        self.i_sc = 0.0
        self.p_th = 0.0
        self.v_mp = 0.0
        self.i_mp = 0.0
        self.p_mp = 0.0
        self.j_sc = 0.0
        self.r_th = 0.0
        self.j_mp = 0.0
        self.r_mp = 0.0
        self.fill = 0.0
        self.ifit = None
        self.pfit = None
        self.jfit = None
        self.rfit = None
        if kwargs:
            self.calculate(**kwargs)

    def summary(self):
        """
        Get summary of all calculated results as a dictionary.
        """
        results = IVCurve.summary(self)
        results.update(v_oc=self.v_oc)
        results.update(i_sc=self.i_sc)
        results.update(p_th=self.p_th)
        results.update(v_mp=self.v_mp)
        results.update(i_mp=self.i_mp)
        results.update(p_mp=self.p_mp)
        results.update(j_sc=self.j_sc)
        results.update(r_th=self.r_th)
        results.update(j_mp=self.j_mp)
        results.update(r_mp=self.r_mp)
        results.update(fill=self.fill)
        return results

    def calculate(self, mode='interp1d', recycle=False, **kwargs):
        """
        Calculate fill factor.  kwargs are passed to fitting functions.

        :param mode: fitting mode (tanh, power, interp1d, spline, erf)
        :param recycle: recycle fit parameters when guessing

        :type mode: str
        :type recycle: bool

        ============ ============
        Mode         Option
        ============ ============
        **power**    order=8
        **tanh**
        **erf**
        **interp1d** kind='cubic'
        **spline**   k=5
        ============ ============

        >>> ivcurve = lm.ivcurve.IVCurve.load_pkl('gathered.pkl.gz')
        >>> ivcurve.calculate(mode='interp1d', kind='linear')
        >>> print ivcurve.fill
        """
        if mode == 'power':
            self._fit_power(recycle=recycle, **kwargs)

        elif mode == 'tanh':
            self._fit_tanh(recycle=recycle, **kwargs)

        elif mode == 'erf':
            self._fit_erf(recycle=recycle, **kwargs)

        elif mode == 'interp1d':
            self._fit_interp1d(recycle=recycle, **kwargs)

        elif mode == 'spline':
            self._fit_spline(recycle=recycle, **kwargs)

        else:
            raise NotImplementedError('unknown fit mode: %s' % mode)

        ### >>> units on
        self.v_oc = self.ifit.solve(x0=self.v_shift) * self.units.v
        self.i_sc = self.ifit(0) * self.units.i
        self.p_th =(self.v_oc * self.i_sc).to(self.units.p)

        self.v_mp = self.pfit.minimize(0.66 * self.v_oc.magnitude,
            return_y=False) * self.units.v
        self.i_mp = self.ifit(self.v_mp.magnitude) * self.units.i
        self.p_mp =(self.i_mp * self.v_mp).to(self.units.p)

        area = np.mean(self.a) * self.units.a

        self.j_sc = (self.i_sc / area).to(self.units.j)
        self.j_mp = (self.i_mp / area).to(self.units.j)

        self.r_th = (self.p_th / area).to(self.units.r)
        self.r_mp = (self.p_mp / area).to(self.units.r)

        self.fill = (self.p_mp / self.p_th * 100)

        self.fill = float(self.fill.magnitude)
        self.v_oc = float(self.v_oc.magnitude)
        self.i_sc = float(self.i_sc.magnitude)
        self.p_th = float(self.p_th.magnitude)
        self.v_mp = float(self.v_mp.magnitude)
        self.i_mp = float(self.i_mp.magnitude)
        self.p_mp = float(self.p_mp.magnitude)
        self.j_sc = float(self.j_sc.magnitude)
        self.r_th = float(self.r_th.magnitude)
        self.j_mp = float(self.j_mp.magnitude)
        self.r_mp = float(self.r_mp.magnitude)
        ### <<< units off

    def _fit_power(self, recycle=False, order=8, **kwargs):
        """
        Fit using power series.
        """
        self.ifit = lm.fit.FitPower(self.v, self.i,
            popt=None, yerr=self.ierr, order=order)

        self.jfit = lm.fit.FitPower(self.v, self.j,
            popt=None, yerr=self.jerr, order=order)

        if recycle:
            p_popt = self.ifit.popt
            r_popt = self.jfit.popt
        else:
            p_popt = None
            r_popt = None

        order += 1

        self.pfit = lm.fit.FitPower(self.v, self.p,
            popt=p_popt, yerr=self.perr, order=order)

        self.rfit = lm.fit.FitPower(self.v, self.r,
            popt=r_popt, yerr=self.rerr, order=order)

    def _fit_tanh(self, recycle=False, **kwargs):
        """
        Fit using hyperbolic tangent.
        """
        i_popt = [self.stats.i.min, 1, -self.v_shift, 0]
        j_popt = [self.stats.j.min, 1, -self.v_shift, 0]

        self.ifit = lm.fit.FitTanh(self.v, self.i, popt=i_popt,
            yerr=self.ierr)

        self.jfit = lm.fit.FitTanh(self.v, self.j, popt=j_popt,
            yerr=self.jerr)

        if recycle:
            p_popt = self.ifit.popt
            r_popt = self.jfit.popt
        else:
            p_popt = i_popt
            r_popt = j_popt

        self.pfit = lm.fit.FitXTanh(self.v, self.p, popt=p_popt,
            yerr=self.perr)

        self.rfit = lm.fit.FitXTanh(self.v, self.r, popt=r_popt,
            yerr=self.rerr)

    def _fit_erf(self, recycle=False, **kwargs):
        """
        Fit using error function.
        """
        i_popt = [self.stats.i.min, 1, -self.v_shift, 0]
        j_popt = [self.stats.j.min, 1, -self.v_shift, 0]

        self.ifit = lm.fit.FitErf(self.v, self.i, popt=i_popt,
            yerr=self.ierr)

        self.jfit = lm.fit.FitErf(self.v, self.j, popt=j_popt,
            yerr=self.jerr)

        if recycle:
            p_popt = self.ifit.popt
            r_popt = self.jfit.popt
        else:
            p_popt = i_popt
            r_popt = j_popt

        self.pfit = lm.fit.FitXErf(self.v, self.p, popt=p_popt,
            yerr=self.perr)

        self.rfit = lm.fit.FitXErf(self.v, self.r, popt=r_popt,
            yerr=self.rerr)

    def _fit_interp1d(self, recycle=False, kind='cubic', **kwargs):
        """
        Fit using an interpolating polynomial.
        """
        self.ifit = lm.fit.FitInterp1D(self.v, self.i, popt=None,
            yerr=self.ierr, kind=kind)

        self.jfit = lm.fit.FitInterp1D(self.v, self.j, popt=None,
            yerr=self.jerr, kind=kind)

        self.pfit = lm.fit.FitInterp1D(self.v, self.p, popt=None,
            yerr=self.perr, kind=kind)

        self.rfit = lm.fit.FitInterp1D(self.v, self.r, popt=None,
            yerr=self.rerr, kind=kind)

    def _fit_spline(self, recycle=False, k=3, **kwargs):
        """
        Fit using spline.
        """
        self.ifit = lm.fit.FitUnivariateSpline(self.v, self.i,
            popt=None, yerr=self.ierr, k=k)

        self.jfit = lm.fit.FitUnivariateSpline(self.v, self.j,
            popt=None, yerr=self.jerr, k=k)

        self.pfit = lm.fit.FitUnivariateSpline(self.v, self.p,
            popt=None, yerr=self.perr, k=k)

        self.rfit = lm.fit.FitUnivariateSpline(self.v, self.r,
            popt=None, yerr=self.rerr, k=k)

    def __str__(self):
        s = StringIO.StringIO()
        print >> s, '[IVCurveSolar]'
        print >> s, '  {self.string.v_oc} = {self.v_oc:{fmt}} {self.string.v}'
        print >> s, '  {self.string.v_mp} = {self.v_mp:{fmt}} {self.string.v}'
        print >> s, '  {self.string.i_sc} = {self.i_sc:{fmt}} {self.string.i}'
        print >> s, '  {self.string.i_mp} = {self.i_mp:{fmt}} {self.string.i}'
        print >> s, '  {self.string.p_th} = {self.p_th:{fmt}} {self.string.p}'
        print >> s, '  {self.string.p_mp} = {self.p_mp:{fmt}} {self.string.p}'
        print >> s, '  {self.string.fill} = {self.fill:{fmt}} {self.string.f}'
        return s.getvalue().format(self=self, fmt='12.8f')
