# -*- coding: utf-8 -*-
"""
.. note::
    Functions for fitting 2D data.

.. moduleauthor:: Adam Gagorik <adam.gagorik@gmail.com>
"""
import scipy.interpolate as _interpolate
import scipy.optimize as _optimize
import scipy.special as _special
import scipy.misc as _misc
import matplotlib.pyplot as _plt
import numpy as _np

class Fit:
    """
    Base class for fitting.

    :param x: array of x-values
    :param y: array of y-values
    :param func: function object
    :param popt: initial guess for parameters

    :type x: list
    :type y: list
    :type func: function
    :type popt: list

    .. warning:: Do not explicitly create Fit instance, its a base class.

    >>> fit = Fit(xdata, ydata, func)
    >>> x = np.linspace(0, 10, 100)
    >>> y = fit(x)
    >>> plt.plot(x, y, 'r-')
    """

    def __init__(self, x, y, func, popt=None, yerr=None):
        """
        """
        self.x = _np.array(x)
        self.y = _np.array(y)
        self.r2 = 0.0
        self.func = func
        self.popt = popt
        self.yerr = yerr
        if not _np.count_nonzero(self.yerr):
            self.yerr = None

    def plot(self, xmin=None, xmax=None, xpoints=100, **kwargs):
        """
        Wrapper around :func:`matplotlib.pyplot.plot`.  Plots the fit line.

        :param xmin: min x-value
        :param xmax: max x-value
        :param xpoints: number of xpoints

        :type xmax: float
        :type xmin: float
        :type xpoints: int

        >>> fit.plot(xmin=-1.0, xmax=1.0, xpoints=10, lw=2, color='blue')
        """
        if xmin is None:
            xmin = _np.amin(self.x)
        if xmax is None:
            xmax = _np.amax(self.x)
        xs = _np.linspace(xmin, xmax, xpoints)
        ys = self(xs)
        _kwargs = dict(color='k', lw=1, ls='-')
        _kwargs.update(**kwargs)
        _plt.plot(xs, ys, **_kwargs)

    def text(self, s, x, y=None, transform=None, rotate=False,
             draw_behind=True, **kwargs):
        """
        Wrapper around :func:`matplotlib.pyplot.text`.  Useful for putting
        text along the fit line drawn by :func:`Fit.plot`, and rotating the
        text using the derivative.

        :param s: text
        :param x: x-value of text
        :param y: y-value of text; if None, use the fit line
        :param transform: matplotlib transform; if None, plt.gca().transAxes
        :param rotate: rotate text using angle computed from derivative
        :param draw_behind: hide the plot objects behind the text

        :type s: str
        :type x: float
        :type y: float
        :type transform: :class:`matplotlib.transforms.Transform`
        :type rotate: bool
        :type draw_behind: bool

        >>> fit.text('Hello!', 0.1, rotate=True, fontsize='large')
        """
        if transform is None:
            transform = _plt.gca().transAxes
        tDI = _plt.gca().transData.inverted()
        x = tDI.transform(transform.transform((x, 1)))[0]
        if y is None:
            y = self(x)
        else:
            y = tDI.transform(transform.transform((1, y)))[1]
        _kwargs = dict(va='center', ha='center', fontsize='x-small',
                       bbox=dict(alpha=0))
        _kwargs.update(**kwargs)
        if rotate and not _kwargs.has_key('rotation'):
            a = _np.rad2deg(_np.arctan2(self.derivative(x), 1)) * 0.9
            a = _np.array((a,))
            p = _np.array([(x, y)])
            r = _plt.gca().transData.transform_angles(a, p)[0]
            _kwargs['rotation'] = r
        text = _plt.text(x, y, s, **_kwargs)
        if draw_behind:
            _plt.draw()
            extent = text.get_window_extent()
            x0 = extent.x0
            x1 = x0 + extent.width
            x0 = _plt.gca().transData.inverted().transform((x0, 1))[0]
            x1 = _plt.gca().transData.inverted().transform((x1, 1))[0]
            _line_kwargs = dict(color='w', lw=3, ls='-')
            if isinstance(draw_behind, dict):
                _line_kwargs.update(**draw_behind)
            xs = _np.linspace(x0, x1, 10)
            _plt.plot(xs, self(xs), **_line_kwargs)

    def solve(self, y=0, x0=0, return_y=False, **kwargs):
        """
        Wrapper around :func:`scipy.optimize.fsolve`.  Solves y=fit(x) for x.

        :param y: y-value
        :param x0: initial guess
        :param return_y: return x and fit(x)

        :returns: xval, yval

        :type y: float
        :type x0: float
        :type return_y: bool

        >>> xval, yval = fit.solve(y=0, x0=1.0)
        """
        _kwargs = dict()
        _kwargs.update(**kwargs)
        if y == 0:
            x = _optimize.fsolve(self.__call__, x0, **_kwargs)

        else:
            f = lambda x : self.__call__(x) + y
            x = _optimize.fsolve(f, x0, **_kwargs)
        if return_y:
            return x[0], self(x[0])
        return x[0]

    def brute(self, a=0, b=1, find_max=False, return_y=False, **kwargs):
        """
        Wrapper around :func:`scipy.optimize.brute`.  Finds minimum in range.

        :param a: lower x-bound
        :param b: upper x-bound
        :param find_max: find max instead of min
        :param return_y: return x and fit(x)

        :returns: xval, yval

        :type a: float
        :type b: float
        :type find_max: bool
        :type return_y: bool

        >>> xval, yval = fit.brute(a=-1, b=1)
        """
        if find_max:
            func = lambda x : -1.0 * self.__call__(x)
        else:
            func = self.__call__
        _kwargs = dict(func=func, ranges=[(a, b)])
        _kwargs.update(**kwargs)
        x = float(_optimize.brute(**_kwargs)[0])
        if return_y:
            return float(x), float(self(x))
        return float(x)

    def maxbrute(self, a=0, b=1, return_y=False, **kwargs):
        """Calls :func:`Fit.brute` with find_max=True"""
        return self.brute(a, b, True, return_y, **kwargs)

    def minimize(self, x0=0, find_max=False, return_y=True, **kwargs):
        """
        Wrapper around :func:`scipy.optimize.minimize`.  Finds minimum using
        a initial guess.

        :param x0: initial guess for x
        :param find_max: find max instead of min
        :param return_y: return x and fit(x)

        :returns: xval, yval

        :type x0: float
        :type find_max: bool
        :type return_y: bool

        >>> xval, yval = fit.minimize(x0=0.1)
        """
        if find_max:
            func = lambda x : -1.0 * self.__call__(x)
        else:
            func = self.__call__
        if hasattr(_optimize, 'minimize'):
            _kwargs = dict(method='BFGS', options=dict(gtol=1e-2, disp=False))
            _kwargs.update(**kwargs)
            x = _optimize.minimize(func, x0, **_kwargs).x[0]
        else:
            _kwargs = dict(gtol=1e-2, disp=False)
            _kwargs.update(**kwargs)
            x = _optimize.fmin_bfgs(func, x0, **_kwargs)[0]
        if return_y:
            return float(x), float(self(x))
        return float(x)

    def maximize(self, x0, return_y=True, **kwargs):
        """Calls :func:`Fit.minimize` with find_max=True"""
        return self.minimize(x0, True, return_y, **kwargs)

    def derivative(self, x, **kwargs):
        """
        Wrapper around :func:`scipy.misc.derivative`, unless an inheriting
        class implements the derivative analytically.

        :param x: x-value to evaluate derivative at
        :type x: float

        :returns: float
        """
        _kwargs = dict(order=5, dx=0.01)
        _kwargs.update(**kwargs)
        m = _misc.derivative(self.__call__, x, **_kwargs)
        return m

    def tangent(self, x, **kwargs):
        """
        Compute a tangent line at x.

        :param x: x-value to evaluate derivative at
        :param xmin: min x-value
        :param xmax: max x-value
        :param xpoints: number of xpoints

        :type x: float
        :type xmax: float
        :type xmin: float
        :type xpoints: int

        >>> func = fit.tangent(x=1.5)
        >>> print func(1.5)
        ... 0.0
        """
        y = self(x)
        m = self.derivative(x, **kwargs)
        b = y - m * x
        return _np.poly1d([m, b])

    def plot_tangent(self, x, xmin=None, xmax=None, xpoints=100, **kwargs):
        """
        Plot a tangent line at x.

        :param x: x-value to evaluate derivative at
        :param xmin: min x-value
        :param xmax: max x-value
        :param xpoints: number of xpoints

        :type x: float
        :type xmax: float
        :type xmin: float
        :type xpoints: int

        >>> fit.plot_tangent(x=1.5, lw=2, color='blue')
        """
        f = self.tangent(x)
        _kwargs = dict(color='k', lw=2, ls='-')
        _kwargs.update(**kwargs)
        if xmin is None:
            xmin = _np.amin(self.x)
        if xmax is None:
            xmax = _np.amax(self.x)
        xs = _np.linspace(xmin, xmax, xpoints)
        _plt.plot(xs, f(xs), **_kwargs)

    def summary(self):
        """
        Create a summary :py:obj:`dict` of fit results.

        :returns: :py:obj:`dict`
        """
        return dict(popt=self.popt, r2=self.r2)

    def sort(self, **kwargs):
        """
        Sort x and y values.
        """
        self.x = list(self.x)
        self.y = list(self.y)
        self.x, self.y = zip(*sorted(zip(self.x, self.y), key=lambda x : x[0]))
        self.x = _np.array(self.x)
        self.y = _np.array(self.y)

    def _fit(self):
        """perform fit"""
        popt, pcov = _optimize.curve_fit(self.func, self.x, self.y, self.popt,
                                         self.yerr)
        self.popt = popt
        setattr(self, '__call__', lambda x: self.func(x, *self.popt))
        self._calculate_rsquared()

    def _calculate_rsquared(self):
        """calculate correlation coeff"""
        ym  = _np.average(self.y)
        yi  = _np.asanyarray(self.y)
        fi  = _np.asanyarray(self.x)
        ssr = _np.sum((yi - fi) ** 2)
        sst = _np.sum((yi - ym) ** 2)
        self.r2 = 1.0 - ssr / sst
        #A = _np.average(self.y)
        #N = _np.sum((self(self.x) - A)**2)
        #D = _np.sum((self.y - A)**2)
        #print N/D
        #self.r2 = N/D

    def __call__(self, *args, **kwargs):
        return None

    def __str__(self):
        d = self.summary()
        s = ', '.join(['%s=%s' % (k, v) for k, v in d.iteritems()])
        return 'Fit(%s)' % s

class FitPower(Fit):
    r""":math:`\sum_{i=0}^{N} c_i x^i`

    :param order: order of polynomial
    :type order: int
    """
    def __init__(self, x, y, order=1, popt=None, yerr=None):
        Fit.__init__(self, x, y, None, popt, yerr)
        self._fit(order)

    def _fit(self, order):
        self.popt = _np.polyfit(self.x, self.y, order, w=self.yerr)
        self.func = _np.poly1d(self.popt)
        setattr(self, '__call__', self.func)
        setattr(self, 'derivative', self.func.deriv())
        self._calculate_rsquared()

    def summary(self):
        """create a summary :py:obj:`dict`"""
        d = {'c%d' % i : v for i, v in enumerate(self.popt)}
        d['r2'] = self.r2
        return d

class FitLinear(FitPower):
    r""":math:`m x + b`"""
    def __init__(self, x, y, popt=None, yerr=None):
        FitPower.__init__(self, x, y, 1, popt, yerr)
        self.m = float(self.popt[0])
        self.b = float(self.popt[1])
        self.a = _np.rad2deg(_np.arctan2(self.m, 1))
        self.elabel = r'$\mathtt{y=%g x %+g}$' % (self.m, self.b)
        self.alabel = r'$\mathtt{%.2f^{\circ}}$' % (self.a)
        self.mlabel = r'$\mathtt{%g}$' % self.m

    def derivative(self, x, **kwargs):
        """analytic derivative of function"""
        return self.m

    def summary(self):
        """create a summary :py:obj:`dict`"""
        return dict(m=self.m, b=self.b, a=self.a, r2=self.r2)

class FitQuadratic(FitPower):
    r""":math:`a x^{2} + b x + c`"""
    def __init__(self, x, y, popt=None, yerr=None):
        FitPower.__init__(self, x, y, 2, popt, yerr)
        self.a = self.popt[0]
        self.b = self.popt[1]
        self.c = self.popt[2]

    def derivative(self, x, **kwargs):
        """analytic derivative of function"""
        return 2 * self.a * x + self.b

    def summary(self):
        """create a summary :py:obj:`dict`"""
        return dict(a=self.a, b=self.b, c=self.c, r2=self.r2)

class FitInterp1D(Fit):
    """
    Fit to interpolating function.

    :param kind: linear, nearest, zero, slinear, quadratic, cubic.
    :type kind: str
    """
    def __init__(self, x, y, popt=None, yerr=None, **kwargs):
        """
        kind : linear, nearest, zero, slinear, quadratic, cubic
        """
        Fit.__init__(self, x, y, None, None, yerr)
        self._fit(**kwargs)
        self.xmin = _np.amin(x)
        self.xmax = _np.amax(x)

    def _fit(self, **kwargs):
        _kwargs = dict(kind='cubic', fill_value=_np.nan, bounds_error=False)
        _kwargs.update(**kwargs)
        self.func = _interpolate.interp1d(self.x, self.y, **_kwargs)
        setattr(self, '__call__', self.func)
        self._calculate_rsquared()

class FitUnivariateSpline(Fit):
    """
    Fit to spline.

    :param k: degree of spline (<=5)
    :type kind: int
    """
    def __init__(self, x, y, popt=None, yerr=None, **kwargs):
        Fit.__init__(self, x, y, None, None, yerr)
        self._fit(**kwargs)
        self.xmin = _np.amin(x)
        self.xmax = _np.amax(x)

    def _fit(self, **kwargs):
        _kwargs = dict(k=5)
        _kwargs.update(**kwargs)
        self.func = _interpolate.UnivariateSpline(self.x, self.y, **_kwargs)
        setattr(self, '__call__', self.func)
        self._calculate_rsquared()

class FitLagrange(Fit):
    """
    Fit to lagrange interpolating spline.  It sucks.
    """
    def __init__(self, x, y, popt=None, yerr=None):
        Fit.__init__(self, x, y, None, None, yerr)
        self._fit()
        self.xmin = _np.amin(x)
        self.xmax = _np.amax(x)

    def _fit(self):
        self.func = _interpolate.lagrange(self.x, self.y)
        setattr(self, '__call__', self.func)
        self._calculate_rsquared()

class FitBarycentric(Fit):
    """
    Fit to lagrange interpolating spline.  It sucks.
    """
    def __init__(self, x, y, popt=None, yerr=None, **kwargs):
        Fit.__init__(self, x, y, None, None, yerr)
        self._fit(**kwargs)
        self.xmin = _np.amin(x)
        self.xmax = _np.amax(x)

    def _fit(self, **kwargs):
        self.func = self.__call__
        self._calculate_rsquared()

    def __call__(self, x):
        return _interpolate.barycentric_interpolate(self.x, self.y, x)

class FitTanh(Fit):
    r""":math:`a \tanh(b x + c) + d`"""
    def __init__(self, x, y, popt=None, yerr=None):

        def func(x, a=1, b=1, c=0, d=0):
            return a * _np.tanh(b * x + c) + d

        Fit.__init__(self, x, y, func, popt, yerr)

        self._fit()
        self.a = self.popt[0]
        self.b = self.popt[1]
        self.c = self.popt[2]
        self.d = self.popt[3]

    def derivative(self, x, **kwargs):
        """analytic derivative of function"""
        v = _np.tanh(self.b * x + self.c)
        return self.a * self.b * (1.0 - v**2)

    def summary(self):
        """create a summary :py:obj:`dict`"""
        return dict(a=self.a, b=self.b, c=self.c, d=self.d, r2=self.r2)

class FitLog(Fit):
    r""":math:`a \tanh(b x + c) + d`"""
    def __init__(self, x, y, popt=None, yerr=None):

        def func(x, a=1, b=1, c=0, d=0):
            return a * _np.log(b * x + c) + d

        Fit.__init__(self, x, y, func, popt, yerr)

        self._fit()
        self.a = self.popt[0]
        self.b = self.popt[1]
        self.c = self.popt[2]
        self.d = self.popt[3]

    def derivative(self, x, **kwargs):
        """analytic derivative of function"""
        return self.a * self.b * (self.b * x + self.c)

    def summary(self):
        """create a summary :py:obj:`dict`"""
        return dict(a=self.a, b=self.b, c=self.c, d=self.d, r2=self.r2)

class FitXTanh(Fit):
    r""":math:`a x \tanh(b x + c) + d`"""
    def __init__(self, x, y, popt=None, yerr=None):

        def func(x, a=1, b=1, c=0, d=0):
            return a * x * _np.tanh(b * x + c) + d

        Fit.__init__(self, x, y, func, popt, yerr)

        self._fit()
        self.a = self.popt[0]
        self.b = self.popt[1]
        self.c = self.popt[2]
        self.d = self.popt[3]

    def derivative(self, x, **kwargs):
        """analytic derivative of function"""
        v = _np.tanh(self.b * x + self.c)
        return self.a * v + self.a * x * self.b * (1.0 - v**2)

    def summary(self):
        """create a summary :py:obj:`dict`"""
        return dict(a=self.a, b=self.b, c=self.c, d=self.d, r2=self.r2)

class FitErf(Fit):
    r""":math:`a\,\mathrm{erf}(b x + c) + d`"""
    def __init__(self, x, y, popt=None, yerr=None):

        def func(x, a=1, b=1, c=0, d=0):
            return a * _special.erf(b * x + c) + d

        Fit.__init__(self, x, y, func, popt, yerr)

        self._fit()
        self.a = self.popt[0]
        self.b = self.popt[1]
        self.c = self.popt[2]
        self.d = self.popt[3]

    def summary(self):
        """create a summary :py:obj:`dict`"""
        return dict(a=self.a, b=self.b, c=self.c, d=self.d, r2=self.r2)

class FitXErf(Fit):
    r""":math:`a x\,\mathrm{erf}(b x + c) + d`"""
    def __init__(self, x, y, popt=None, yerr=None):

        def func(x, a=1, b=1, c=0, d=0):
            return a * x * _special.erf(b * x + c) + d

        Fit.__init__(self, x, y, func, popt, yerr)

        self._fit()
        self.a = self.popt[0]
        self.b = self.popt[1]
        self.c = self.popt[2]
        self.d = self.popt[3]

    def summary(self):
        """create a summary :py:obj:`dict`"""
        return dict(a=self.a, b=self.b, c=self.c, d=self.d, r2=self.r2)

class FitGaussian(Fit):
    r""":math:`a e^{\frac{-(x - m)^{2}}{2 \sigma^{2}}}`"""
    def __init__(self, x, y, popt=None, yerr=None):

        def func(x, a=1, m=0, s=1):
            return a * _np.exp(-(x - m)**2 / (2.0 * s * s))

        Fit.__init__(self, x, y, func, popt, yerr)

        self._fit()
        self.a = self.popt[0]
        self.m = self.popt[1]
        self.s = self.popt[2]

    def summary(self):
        """create a summary :py:obj:`dict`"""
        return dict(a=self.a, m=self.m, s=self.s, r2=self.r2)

class FitSin(Fit):
    r""":math:`a \sin(b x + c) + d`"""
    def __init__(self, x, y, popt=None, yerr=None):

        def func(x, a=1, b=1, c=0, d=0):
            return a * _np.sin(b * x + c) + d

        Fit.__init__(self, x, y, func, popt, yerr)

        self._fit()
        self.a = self.popt[0]
        self.b = self.popt[1]
        self.c = self.popt[2]
        self.d = self.popt[3]

    def derivative(self, x, **kwargs):
        """analytic derivative of function"""
        return self.a * self.b * _np.cos(self.b * x + self.c)

    def summary(self):
        """create a summary :py:obj:`dict`"""
        return dict(a=self.a, b=self.b, c=self.c, d=self.d, r2=self.r2)

class FitCos(Fit):
    r""":math:`a \cos(b x + c) + d`"""
    def __init__(self, x, y, popt=None, yerr=None):

        def func(x, a=1, b=1, c=0, d=0):
            return a * _np.cos(b * x + c) + d

        Fit.__init__(self, x, y, func, popt, yerr)

        self._fit()
        self.a = self.popt[0]
        self.b = self.popt[1]
        self.c = self.popt[2]
        self.d = self.popt[3]

    def derivative(self, x, **kwargs):
        """analytic derivative of function"""
        return -1.0 * self.a * self.b * _np.sin(self.b * x + self.c)

    def summary(self):
        """create a summary :py:obj:`dict`"""
        return dict(a=self.a, b=self.b, c=self.c, d=self.d, r2=self.r2)

class FitLinearZero(Fit):
    r""":math:`mx`"""
    def __init__(self, x, y, popt=None, yerr=None):

        def func(x, m=1):
            return m * x

        Fit.__init__(self, x, y, func, popt, yerr)

        self._fit()
        self.m = self.popt[0]

    def derivative(self, x, **kwargs):
        """analytic derivative of function"""
        return self.m

    def summary(self):
        """create a summary :py:obj:`dict`"""
        return dict(self.m, r2=self.r2)

class FitQuadraticZero(Fit):
    r""":math:`mx`"""
    def __init__(self, x, y, popt=None, yerr=None):

        def func(x, a=1, b=1):
            return a * x * x + b * x

        Fit.__init__(self, x, y, func, popt, yerr)

        self._fit()
        self.a = self.popt[0]
        self.b = self.popt[1]

    def derivative(self, x, **kwargs):
        """analytic derivative of function"""
        return 2 * self.a * x + self.b

    def summary(self):
        """create a summary :py:obj:`dict`"""
        return dict(self.a, self.b, r2=self.r2)
