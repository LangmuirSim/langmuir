# -*- coding: utf-8 -*-
from langmuir.common import ifactor
import numpy as np
import langmuir
import pickle

def create(rows=0):
    """
    create dat array with values set to zero

    rows: number of rows
    """
    return np.zeros(shape=(rows,), dtype=langmuir.database.output.dtype)

def load_dat(handle):
    """
    load dat file into ndarray
    """
    if isinstance(handle, str):
        handle = langmuir.common.zhandle(handle, 'rb')

    titles = [t.strip() for t in handle.readline().strip().split()]
    values = np.loadtxt(handle, dtype=float)

    data = create(values.shape[0])
    for i, title in enumerate(titles):
        data[title] = values[:,i]

    return np.sort(data, order='simulation:time')

def load_pkl(handle):
    """
    load pkl file into ndarray
    """
    if isinstance(handle, str):
        handle = langmuir.common.zhandle(handle, 'rb')
    obj = pickle.load(handle)
    return obj

def save_pkl(obj, handle):
    """
    save ndarray into pkl file
    """
    if isinstance(handle, str):
        handle = langmuir.common.zhandle(handle, 'wb')
    pickle.dump(obj, handle, pickle.HIGHEST_PROTOCOL)

def combine_first(A, B, index=None):
    """
    combine two tables with same dtype
    """
    assert A.dtype == B.dtype

    if index is None:
        return np.hstack((A, B))

    assert index in A.dtype.names

    idxA = set(A[index])
    if not A.size == len(idxA):
        raise RuntimeError('index of A contains duplicates')

    idxB = set(B[index])
    if not B.size == len(idxB):
        raise RuntimeError('index of B contains duplicates')

    idxU = idxA.union(idxB)
    rows = len(idxU)

    if A.size + B.size == rows:
        return np.sort(np.hstack((A, B)), order=index)

    C = np.zeros(shape=(rows,), dtype=A.dtype)
    mapA = {A[i][index] : i for i in range(A.size)}
    mapB = {B[i][index] : i for i in range(B.size)}

    for j, i in enumerate(idxU):
        inA = i in idxA
        inB = i in idxB
        if inA and inB:
            C[j] = A[mapA[i]]
        elif inA:
            C[j] = A[mapA[i]]
        elif inB:
            C[j] = B[mapB[i]]
        else:
            raise RuntimeError('invalid index in union of A and B')

    return C

def combine(objs, load_func=load_dat):
    """
    combine a list of dat files, assumed to be indexed by simulation:time

    objs: list of file names, or ndarrays
    load_func: function used to open files
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

        obj['part'] = i

        if frame is None:
            frame = obj
        else:
            real = frame['real:time'][-1]
            obj['real:time'] += real
            frame = combine_first(obj, frame, index='simulation:time')

    return frame

def calculate(data):
    """
    calculate current, etc for ndarray
    """
    t = data['simulation:time'].astype(float)
    for flux in langmuir.database.fluxes:
        a = flux + ':attempt'
        s = flux + ':success'
        p = flux + ':probability'
        r = flux + ':rate'
        c = flux + ':current'
        data[p] = data[s] / data[a].astype(float) * 100
        data[r] = data[s] / t
        data[c] = data[r] * ifactor

    data['drain:current'] = \
        data['eDrainR:current'] - data['eDrainL:current'] + \
        data['hDrainL:current'] - data['hDrainR:current']

    data['source:current'] = \
        data['eSourceR:current'] - data['eSourceL:current'] + \
        data['hSourceL:current'] - data['hSourceR:current']

    data['carrier:count'] = data['electron:count'] + data['hole:count']
    data['carrier:difference'] = data['electron:count'] - \
        data['hole:count']

    return data

def equilibrate(data, i1=None, i0=None):
    """
    extract result, taking out equilibration steps

    data: ndarray
    i1: int, index of "last step"
    i0: int, index of "equilibration step"
    """
    if i0 is None or i0 < -data.size:
        if i1 is None:
            return data
        return data[i1]
    t0 = data[i0]
    data['simulation:time'] -= t0['simulation:time']
    data['real:time'] -= t0['real:time']
    for flux in langmuir.database.fluxes:
        a = flux + ':attempt'
        s = flux + ':success'
        data[a] -= t0[a]
        data[s] -= t0[s]
    data = calculate(data)
    if i1 is None:
        return data
    return data[i1]

def add_field(a, name, fmt, before=False):
    """
    Add a field to ndarray

    a: ndarray
    name: field name
    fmt: field type
    before: add it before the other fields, otherwise after
    """
    if a.dtype.fields is None:
        raise ValueError, "not a structured numpy array"
    if before:
        dtype=[(name, fmt)] + a.dtype.descr
    else:
        dtype=a.dtype.descr + [(name, fmt)]
    b = np.empty(a.shape, dtype)
    for name in a.dtype.names:
        b[name] = a[name]
    return b