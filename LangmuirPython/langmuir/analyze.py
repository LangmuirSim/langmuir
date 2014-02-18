# -*- coding: utf-8 -*-
"""
@author: adam
"""
import langmuir as lm
import pandas as pd

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