# -*- coding: utf-8 -*-
"""
.. note::
    Functions for analyzing Langmuir output.

    1.  Combine each part of a simulation with :py:func:`combine`
    2.  Calculate the current with :py:func:`calculate`
    3.  Extract the result with :py:func:`equilibrate`
    4.  Average over runs with :py:func:`create_panel`

.. seealso::

    1.  :download:`combine.py <../../analyze/combine.py>`
    2.  :download:`gather.py <../../analyze/gather.py>`

.. moduleauthor:: Adam Gagorik <adam.gagorik@gmail.com>
"""
import langmuir as lm
import pandas as pd
import numpy as np
import collections
import StringIO

fluxes = ['eSourceL', 'eSourceR', 'hSourceL', 'hSourceR', 'eDrainL', 'eDrainR',
    'hDrainL', 'hDrainR', 'xSource', 'xDrain']

try:
    import scipy.constants as constants
    ifactor = constants.e * 1e21
except ImportError:
    ifactor = 160.21764869999996

def create_panel(frames, index=None):
    """
    Turn a list of :py:class:`pandas.DataFrame` into a
    :py:class:`pandas.Panel`.

    :param frames: list of :py:class:`pandas.DataFrame`
    :param index: major axis labels

    :type frames: list
    :type index: list

    >>> data1 = lm.common.load_pkl('run.0/calculated.pkl.gz')
    >>> data2 = lm.common.load_pkl('run.1/calculated.pkl.gz')
    >>> panel = create_panel([data1, data2], index=[0, 1])
    """
    if index is None:
        index = range(len(frames))
    return pd.Panel({i : frame for i, frame in zip(index, frames)})

def combine(objs):
    """
    Combine a set of panda's DataFrames into a single DataFrame.  The idea
    is that each DataFrame holds data from a part of some series of data.
    The index of each part should be the *simulation:time*.

    :param objs: list of :py:class:`pandas.DataFrame`
    :type objs: list

    >>> data1 = lm.datfile.load('part.0/out.dat.gz')
    >>> data2 = lm.datfile.load('part.1/out.dat.gz')
    >>> combined = lm.analyze.combine([data1, data2])
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
    Compute all flux statistics.  Calculates current using, for example,
    the number of carriers exiting a drain.

    :param obj: data
    :type obj: :py:class:`pandas.DataFrame`

    >>> data = lm.common.load_pkl('combined.pkl.gz')
    >>> data = lm.analyze.calculate(data)
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

    :param obj: data
    :param last: index of last step
    :param equil: index of first step

    :type obj: :py:class:`pandas.DataFrame`
    :type last: int
    :type equil: int

    >>> data = lm.common.load_pkl('combined.pkl.gz')
    >>> data = lm.analyze.equilibrate(data, last=-1, equil=-1000)
    """
    last = obj.xs(obj.index[last])
    if equil is None:
        return last
    equil = obj.xs(obj.index[equil])
    return last - equil

class Stats(object):
    """
    Compute various statistics of an array like object.

    ======== ==================
    Attr     Description
    ======== ==================
    **max**  max
    **min**  min
    **rng**  range
    **avg**  average
    **std**  standard deviation
    ======== ==================

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
        self.avg    = np.mean(array)
        self.std    = np.std(array)

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
        return d

    def __str__(self):
        s = StringIO.StringIO()
        print >> s, r'{self.prefix}max  = {self.max:{fmt}}'
        print >> s, r'{self.prefix}min  = {self.min:{fmt}}'
        print >> s, r'{self.prefix}rng  = {self.rng:{fmt}}'
        print >> s, r'{self.prefix}avg  = {self.avg:{fmt}}'
        print >> s, r'{self.prefix}std  = {self.std:{fmt}}'
        return s.getvalue().format(fmt='+.5f', self=self)
