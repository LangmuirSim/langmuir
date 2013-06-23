# -*- coding: utf-8 -*-
from langmuir.common import ifactor
import pickle as _pickle
import pandas as _pd
import langmuir

def fix(obj):
    """
    Fix a panda's DataFrame to have correct types and column names
    """
    columns = langmuir.database.output.names
    obj = obj.fillna(value=0)
    for col in columns:
        if col in obj.columns:
            pytype = langmuir.database.output[col].pytype
            obj[col] = obj[col].astype(pytype)
    return obj

def load_dat(handle, compression=None, sep='\s*', **kwargs):
    """
    Load a Langmuir output dat file into a panda's DataFrame

    handle : file name or obj
    kwargs : keyword arguments passed on to pandas.read_table
    """
    if isinstance(handle, str):
        if handle.endswith('.gz'):
            compression = 'gzip'
    kwargs.update(compression=compression, sep=sep)
    return fix(_pd.read_table(handle, **kwargs))

def load_pkl(handle, max_objs=10):
    """
    Load max objs from a pkl file
    """
    if isinstance(handle, str):
        handle = langmuir.common.zhandle(handle, 'rb')
    objs = []
    for i in range(max_objs + 1):
        if i == max_objs:
            raise RuntimeError('too many objects loaded from pkl file')
        try:
            obj = _pickle.load(handle)
            objs.append(obj)
        except EOFError:
            break
    objs = tuple(objs)
    if not objs:
        raise RuntimeError('no objects in pkl file')
    elif len(objs) == 1:
        return objs[0]
    return objs

def load_pkls(pkls):
    """
    Load a set of pkls into a Panda's Panel
    """
    if isinstance(pkls, str):
        pkls = [pkls]
    panel = {}
    for i, pkl in enumerate(pkls):
        panel[i] = langmuir.analyze.load_pkl(pkl)
    return _pd.Panel(panel)

def save_pkl(obj, handle):
    """
    Save obj to a pkl file
    """
    if isinstance(handle, str):
        handle = langmuir.common.zhandle(handle, 'wb')
    _pickle.dump(obj, handle, _pickle.HIGHEST_PROTOCOL)
    return handle

def combine(objs, load_func=load_dat):
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
            obj = load_func(obj)

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

    return fix(frame)

def calculate_flux(obj, flux):
    """
    Compute current for flux
    """
    a = flux + ':attempt'
    s = flux + ':success'
    p = flux + ':prob'
    r = flux + ':rate'
    c = flux + ':current'
    t = 'simulation:time'
    obj[p] = obj[s] / obj[a].astype(float) * 100.0
    obj[r] = obj[s] / obj[t].astype(float)
    obj[c] = obj[r] * ifactor
    obj = obj.fillna(value=0.0)
    return obj

def calculate_out(obj):
    """
    Compute carrier out flux
    """
    obj['eout:attempt'] = obj['eDrainL:attempt'] + \
                          obj['eDrainR:attempt'] + \
                          obj['xDrain:attempt' ]

    obj['eout:success'] = obj['eDrainL:success'] + \
                          obj['eDrainR:success'] + \
                          obj['xDrain:success' ]

    obj['hout:attempt'] = obj['hDrainL:attempt'] + \
                          obj['hDrainR:attempt'] + \
                          obj['xDrain:attempt' ]

    obj['hout:success'] = obj['hDrainL:success'] + \
                          obj['hDrainR:success'] + \
                          obj['xDrain:success' ]

    obj['eout:attempt'] = obj['eout:attempt'].astype(int)
    obj['eout:success'] = obj['eout:success'].astype(int)
    obj['hout:attempt'] = obj['hout:attempt'].astype(int)
    obj['hout:success'] = obj['hout:success'].astype(int)

    obj = calculate_flux(obj, 'eout')
    obj = calculate_flux(obj, 'hout')
    return obj

def calculate_in(obj):
    """
    Compute carrier in flux
    """
    obj['ein:attempt'] = obj['eSourceL:attempt'] + \
                         obj['eSourceR:attempt'] + \
                         obj['xSource:attempt' ]

    obj['ein:success'] = obj['eSourceL:success'] + \
                         obj['eSourceR:success'] + \
                         obj['xSource:success' ]

    obj['hin:attempt'] = obj['hSourceL:attempt'] + \
                         obj['hSourceR:attempt'] + \
                         obj['xSource:attempt' ]

    obj['hin:success'] = obj['hSourceL:success'] + \
                         obj['hSourceR:success'] + \
                         obj['xSource:success' ]

    obj['ein:attempt'] = obj['ein:attempt'].astype(int)
    obj['ein:success'] = obj['ein:success'].astype(int)
    obj['hin:attempt'] = obj['hin:attempt'].astype(int)
    obj['hin:success'] = obj['hin:success'].astype(int)

    obj = calculate_flux(obj, 'ein')
    obj = calculate_flux(obj, 'hin')
    return obj

def calculate_left(obj):
    """
    Compute carrier left flux
    """
    obj['eleft:attempt'] = obj['eSourceL:attempt'] - obj['eDrainL:attempt']
    obj['eleft:success'] = obj['eSourceL:success'] - obj['eDrainL:success']
    obj['hleft:attempt'] = obj['hSourceL:attempt'] - obj['hDrainL:attempt']
    obj['hleft:success'] = obj['hSourceL:success'] - obj['hDrainL:success']

    obj['eleft:attempt'] = obj['eleft:attempt'].astype(int)
    obj['eleft:success'] = obj['eleft:success'].astype(int)
    obj['hleft:attempt'] = obj['hleft:attempt'].astype(int)
    obj['hleft:success'] = obj['hleft:success'].astype(int)

    obj = calculate_flux(obj, 'eleft')
    obj = calculate_flux(obj, 'hleft')
    return obj

def calculate_right(obj):
    """
    Compute carrier right flux
    """
    obj['eright:attempt'] = obj['eSourceR:attempt'] - obj['eDrainR:attempt']
    obj['eright:success'] = obj['eSourceR:success'] - obj['eDrainR:success']
    obj['hright:attempt'] = obj['hSourceR:attempt'] - obj['hDrainR:attempt']
    obj['hright:success'] = obj['hSourceR:success'] - obj['hDrainR:success']

    obj['eright:attempt'] = obj['eright:attempt'].astype(int)
    obj['eright:success'] = obj['eright:success'].astype(int)
    obj['hright:attempt'] = obj['hright:attempt'].astype(int)
    obj['hright:success'] = obj['hright:success'].astype(int)

    obj = calculate_flux(obj, 'eright')
    obj = calculate_flux(obj, 'hright')
    return obj

def calculate(obj):
    """
    Compute all flux
    """
    for flux in langmuir.database.fluxes:
        obj = calculate_flux(obj, flux)
    obj = calculate_left(obj)
    obj = calculate_right(obj)
    obj = calculate_out(obj)
    obj = calculate_in(obj)

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

    return obj

def equilibrate(obj, last, equil=None):
    """
    Get the difference between two steps
    """
    last = obj.xs(obj.index[last])
    if equil is None:
        return last
    equil = obj.xs(obj.index[equil])
    return last - equil