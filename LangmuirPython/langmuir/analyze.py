# -*- coding: utf-8 -*-
"""
@author: adam
"""
import langmuir as lm
import pandas as pd
import numpy as np
import collections
import StringIO

try:
    import scipy.stats as stats
except ImportError:
    pass

fluxes = ['eSourceL', 'eSourceR', 'hSourceL', 'hSourceR', 'eDrainL', 'eDrainR',
    'hDrainL', 'hDrainR', 'xSource', 'xDrain']

try:
    import quantities as units
    ifactor = float((units.e / units.ps).rescale(units.nA))
except ImportError:
    ifactor = 160.21764869999996

def combine(objs):
    """
    Combine a set of panda's DataFrames into a single DataFrame
    """
    try:
        assert isinstance(objs, list)
    except AssertionError:
        try:
            assert isinstance(objs, tuple)
        except AssertionError:
            objs = [objs]

    frame = None
    for i, obj in enumerate(objs):
        if isinstance(obj, str):
            obj = lm.datfile.load(obj)

        obj = obj.set_index('simulation:time', drop=False)
        obj['part'] = i

        if frame is None:
            frame = obj
        else:
            real = frame['real:time'][frame.index[-1]]
            obj['real:time'] += real
            frame = obj.combine_first(frame)

    frame['new_index'] = range(len(frame))
    frame = frame.set_index('new_index', drop=True)
    frame.index.name = None

    return lm.datfile.fix(frame)

def calculate(obj):
    """
    Compute all flux.
    """
    for flux in fluxes:
        a = flux + ':attempt'
        s = flux + ':success'
        p = flux + ':prob'
        r = flux + ':rate'
        c = flux + ':current'
        t = 'simulation:time'
        obj[p] = obj[s] / obj[a].astype(float) * 100.0
        obj[r] = obj[s] / obj[t].astype(float)
        obj[c] = obj[r] * ifactor

    obj['eDrain:rate'] = obj['eDrainR:rate'] - obj['eDrainL:rate']
    obj['eDrain:current'] = obj['eDrain:rate'] * ifactor

    obj['hDrain:rate'] = obj['hDrainL:rate'] - obj['hDrainR:rate']
    obj['hDrain:current'] = obj['hDrain:rate'] * ifactor

    obj['drain:rate'] = obj['eDrain:rate'] + obj['hDrain:rate']
    obj['drain:current'] = obj['drain:rate'] * ifactor

    obj['eSource:rate'] = obj['eSourceR:rate'] - obj['eSourceL:rate']
    obj['eSource:current'] = obj['eSource:rate'] * ifactor

    obj['hSource:rate'] = obj['hSourceL:rate'] - obj['hSourceR:rate']
    obj['hSource:current'] = obj['hSource:rate'] * ifactor

    obj['source:rate'] = obj['eSource:rate'] + obj['hSource:rate']
    obj['source:current'] = obj['source:rate'] * ifactor

    obj['carrier:count'] = obj['electron:count'] + obj['hole:count']
    obj['carrier:difference'] = obj['electron:count'] - obj['hole:count']

    obj['speed'] = obj['real:time'].diff() / \
        obj['simulation:time'].diff().astype(float)

    obj = obj.fillna(value=0.0)

    return obj

def equilibrate(obj, last, equil=None):
    """
    Get the difference between two steps.
    """
    last = obj.xs(obj.index[last])
    if equil is None:
        return last
    equil = obj.xs(obj.index[equil])
    return last - equil

class Stats(object):
    """
    Compute various statistics of an array like object.

    ======== ==========================
    Attr     Description
    ======== ==========================
    **max**  max of data
    **min**  min of data
    **rng**  range of data
    **avg**  average of data
    **std**  standard deviation of data
    ======== ==========================

    >>> s = Stats([1, 2, 3, 4, 5])
    """
    def __init__(self, array, prefix=''):
        """
        :param array: array like object
        :type array: list
        """
        self.prefix = prefix
        self.max    = np.amax(array)
        self.min    = np.amin(array)
        self.rng    = abs(self.max - self.min)
        self.avg    = np.average(array)
        self.std    = np.std(array)

        try:
            self.skew = stats.skew(array, 0, bias=False)
            self.skew_z, self.skew_p = stats.skewtest(array, 0)

            self.kurt = stats.kurtosis(array, 0, bias=False)
            self.kurt_z, self.kurt_p = stats.kurtosistest(array, 0)

        except:
            self.skew   = 0.0
            self.skew_z = 0.0
            self.skew_p = 0.0

            self.kurt   = 0.0
            self.kurt_z = 0.0
            self.kurt_p = 0.0

    def to_dict(self):
        """
        Get summary of stats.
        """
        d = collections.OrderedDict()
        d['%smax' % self.prefix] = float(self.max)
        d['%smin' % self.prefix] = float(self.min)
        d['%srng' % self.prefix] = float(self.rng)
        d['%savg' % self.prefix] = float(self.max)
        d['%sstd' % self.prefix] = float(self.max)
        d['%sskw' % self.prefix] = float(self.skw)
        d['%sskz' % self.prefix] = float(self.skz)
        d['%sskp' % self.prefix] = float(self.skp)
        return d

    def __str__(self):
        s = StringIO.StringIO()
        print >> s, r'{self.prefix}max  = {self.max:{fmt}}'
        print >> s, r'{self.prefix}min  = {self.min:{fmt}}'
        print >> s, r'{self.prefix}rng  = {self.rng:{fmt}}'
        print >> s, r'{self.prefix}avg  = {self.avg:{fmt}}'
        print >> s, r'{self.prefix}std  = {self.std:{fmt}}'
        print >> s, r'{self.prefix}skew = {self.skew:{fmt}}; p={self.skew_p:{fmt}}'
        print >> s, r'{self.prefix}kurt = {self.kurt:{fmt}}; p={self.kurt_p:{fmt}}'
        return s.getvalue().format(fmt='+.5f', self=self)