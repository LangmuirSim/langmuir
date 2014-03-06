# -*- coding: utf-8 -*-
"""
.. note::
    Utility functions.

.. moduleauthor:: Adam Gagorik <adam.gagorik@gmail.com>
"""
import collections
import pickle
import gzip
import sys
import os
import re

try:
    from scipy.misc import comb
except ImportError:
    def comb(N, k, exact=False):
        bc = [1 for i in range(0,k+1)]
        for j in range(1, N - k + 1):
            for i in range(1, k + 1):
                bc[i] = bc[i-1] + bc[i]
        return bc[k]

def zhandle(handle, mode='rb'):
    """
    Open a file using gzip if necessary.

    :param handle: filename
    :type handle: str
    """
    try:
        if handle.endswith('.gz'):
            handle = gzip.open(handle, mode)
        else:
            handle = open(handle, mode)
    except AttributeError:
        pass
    return handle

def splitext(handle, *args, **kwargs):
    """
    Extract stub and extension from a file object or string.

    :param handle: filename
    :type handle: str
    """
    try:
        return os.path.splitext(handle, *args, **kwargs)
    except AttributeError:
        try:
            return os.path.splitext(handle.name, *args, **kwargs)
        except AttributeError:
            return '', ''

def load_pkl(handle, max_objs=256):
    """
    Load max objs from a pkl file
    """
    handle = zhandle(handle, 'rb')
    objs = []
    for i in range(max_objs + 1):
        if i == max_objs:
            raise RuntimeError('too many objects loaded from pkl file')
        try:
            obj = pickle.load(handle)
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
    Load a set of pkls into a list.
    """
    if isinstance(pkls, str):
        pkls = [pkls]
    panel = []
    for i, pkl in enumerate(pkls):
        panel.append(load_pkl(pkl))
    return panel

def save_pkl(obj, handle):
    """
    Save obj to a pkl file.
    """
    handle = zhandle(handle, 'wb')
    pickle.dump(obj, handle, pickle.HIGHEST_PROTOCOL)
    return handle

def format_string(s, **kwargs):
    return re.sub('_\.', '.', s.format(**kwargs).strip('-_'))

def format_output(s='{stub}_{name}.{ext}', stub='', name='out', ext='dat',
    **kwargs):
    ext = ext.lstrip('.')
    _kwargs = dict(stub=stub, name=name, ext=ext)
    _kwargs.update(**kwargs)
    return format_string(s, **_kwargs)

def compare_dicts(dict1, dict2):
    """
    Compare two dictionaries.
    """
    results = collections.OrderedDict()

    s1, s2 = set(dict1.keys()), set(dict2.keys())
    i, u = s1.intersection(s2), s1.union(s2)

    results['size1'] = len(s1)
    results['size2'] = len(s2)
    results['union'] = len(u)
    results['inter'] = len(i)
    results['valid'] = True

    changed = 0
    for k in i:
        if dict1[k] != dict2[k]:
            if not dict1[k] is None and not dict2[k] is None:
                results[k] = (dict1[k], dict2[k])
                changed += 1

    if not s1 == s2 or not changed == 0:
        results['valid'] = False

    return results

def compare_lists(list1, list2):
    """
    Compare two lists.
    """
    results = collections.OrderedDict()

    s1, s2 = set(list1), set(list2)
    i, u = s1.intersection(s2), s1.union(s2)

    results['size1'] = len(s1)
    results['size2'] = len(s2)
    results['union'] = len(u)
    results['inter'] = len(i)
    results['valid'] = True

    if not s1 == s2:
        results['valid'] = False

    return results

def command_script(paths, name=None, stub='run', command=None):
    """
    Create a handy bash script that loops over the paths.

    :param paths: list of paths
    :param name: name of script
    :param stub: jobname stub
    :param command: command to insert at each directory

    :type paths: list
    :type name: str
    :type stub: str
    :type command: str
    """
    work = os.getcwd()
    if not name is None:
        handle = open(name, 'w')
    else:
        handle = sys.stdout

    print >> handle, r'#!/bin/bash'
    print >> handle, ''
    print >> handle, 'FILE=~/templates/run.batch'
    print >> handle, ''
    print >> handle, 'custom_command ()'
    print >> handle, '{'
    print >> handle, '    echo ""'
    print >> handle, '    echo "%s"' % ('-' * 80)
    print >> handle, '    echo `pwd`'
    print >> handle, '    echo "$1"'
    print >> handle, '    echo "%s"' % ('-' * 80)
    print >> handle, '    echo ""'
    print >> handle, ''
    if not command:
        print >> handle, '#    submitLangmuir $1 sim.inp'
        print >> handle, ''
        print >> handle, '#    cp $FILE .'
        print >> handle, ''
        print >> handle, '#    qsub -N $1 run.batch'
        print >> handle, ''
        print >> handle, '#    langmuir sim.inp'
        print >> handle, ''
        print >> handle, '#    grep "max.threads" sim.inp'
        print >> handle, ''
        print >> handle, "#    sed -i 's/\(grid\.z\s*=\s*\).*$/\\11/' sim.inp"
        print >> handle, ''
        print >> handle, '#    ./submit'
    else:
        print >> handle, command
    print >> handle, ''
    print >> handle, '}'
    print >> handle, ''
    for i, sim in enumerate(paths):
        print >> handle, r'cd %s' % os.path.relpath(sim, work)
        print >> handle, r'custom_command "%s%d"' % (stub, i)
        print >> handle, r'cd %s' % os.path.relpath(work, sim)
        print >> handle, r''
    if not name is None:
        handle.close()
        os.system('chmod +x %s' % name)