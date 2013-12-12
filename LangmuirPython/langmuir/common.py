# -*- coding: utf-8 -*-
import subprocess as _subprocess
import datetime as _datetime
import gzip as _gzip
import numpy as _np
import sys as _sys
import os as _os
import re as _re
import langmuir

try:
    from scipy.constants import epsilon_0, e
    prefactor = e / (4.0 * _np.pi * 3.5 * epsilon_0 * 1e-9)
except ImportError:
    prefactor = 0.411418424298

try:
    from scipy.misc import comb
except ImportError:
    def comb(N, k, exact=False):
        bc = [1 for i in range(0,k+1)]
        for j in range(1, N - k + 1):
            for i in range(1, k + 1):
                bc[i] = bc[i-1] + bc[i]
        return bc[k]

try:
    import quantities as units
    ifactor = float((units.e / units.ps).rescale(units.nA))
except ImportError:
    ifactor = 160.21764869999996

class DictDiffer(object):
    """
    Set of static functions used to compare dictionary keys and values.

    >>> d1 = dict(A=1, B=2, C=3)
    >>> d2 = dict(B=2, C=4, D=5)
    >>> print langmuir.common.DictDiff.addedKeys(d1, d2)
    ... set(['A'])
    """
    @staticmethod
    def keySets(d1, d2):
        """
        Turns keys into sets and calculates their intersection / union.

        :param d1: dictionary 1
        :param d2: dictionary 2

        :type d1: dict
        :type d2: dict

        :returns: (set, set, set, set)
        """
        s1, s2 = set(d1.keys()), set(d2.keys())
        i, u = s1.intersection(s2), s1.union(s2)
        return s1, s2, i, u

    @classmethod
    def addedKeys(cls, d1, d2):
        """
        Calculates keys in d1 not in d2.

        :param d1: dictionary 1
        :param d2: dictionary 2

        :type d1: dict
        :type d2: dict

        :returns: set
        """
        s1, s2, i, u = cls.keySets(d1, d2)
        return s1 - i

    @classmethod
    def addedValues(cls, d1, d2):
        """
        Calculates keys in d1 not in d2 with values.

        :param d1: dictionary 1
        :param d2: dictionary 2

        :type d1: dict
        :type d2: dict

        :returns: dict
        """
        keys = cls.addedKeys(d1, d2)
        return {k : d1[k] for k in keys}

    @classmethod
    def removedKeys(cls, d1, d2):
        """
        Calculates keys in d2 not in d1.

        :param d1: dictionary 1
        :param d2: dictionary 2

        :type d1: dict
        :type d2: dict

        :returns: set
        """
        s1, s2, i, u = cls.keySets(d1, d2)
        return s2 - i

    @classmethod
    def removedValues(cls, d1, d2):
        """
        Calculates keys in d2 not in d1 with values.

        :param d1: dictionary 1
        :param d2: dictionary 2

        :type d1: dict
        :type d2: dict

        :returns: dict
        """
        keys = cls.removedKeys(d1, d2)
        return {k : d2[k] for k in keys}

    @classmethod
    def changedKeys(cls, d1, d2):
        """
        Calculates keys in both that have different values.

        :param d1: dictionary 1
        :param d2: dictionary 2

        :type d1: dict
        :type d2: dict

        :returns: set
        """
        s1, s2, i, u = cls.keySets(d1, d2)
        return set(k for k in i if d2[k] != d1[k])

    @classmethod
    def changedValues(cls, d1, d2, split=False):
        """
        Calculates key : value pairs in both that have different values.

        :param d1: dictionary 1
        :param d2: dictionary 2
        :param split: split result up into seperate dictionaries

        :type d1: dict
        :type d2: dict
        :type split: bool

        :returns: (dict, [dict])
        """
        keys = cls.changedKeys(d1, d2)
        if split:
            return {k : d1[k] for k in keys}, {k : d2[k] for k in keys}
        return {k : (d1[k], d2[k]) for k in keys}

    @classmethod
    def unchangedKeys(cls, d1, d2):
        """
        Calculates keys in both that have the same values.

        :param d1: dictionary 1
        :param d2: dictionary 2

        :type d1: dict
        :type d2: dict

        :returns: set
        """
        s1, s2, i, u = cls.keySets(d1, d2)
        return set(k for k in i if d2[k] == d1[k])

    @classmethod
    def unchangedValues(cls, d1, d2, split=False):
        """
        Calculates key : value pairs in both that the same values.

        :param d1: dictionary 1
        :param d2: dictionary 2
        :param split: split result up into seperate dictionaries

        :type d1: dict
        :type d2: dict
        :type split: bool

        :returns: (dict, [dict])
        """
        keys = cls.unchangedKeys(d1, d2)
        if split:
            return {k : d1[k] for k in keys}, {k : d2[k] for k in keys}
        return {k : (d1[k], d2[k]) for k in keys}

def ztail(fname, n=1):
    """
    Run tail on a gzipped file.

    :param fname: name of file
    :param n: number of lines

    :type fname: str
    :type n: int
    """
    proc1 = _subprocess.Popen(['zcat', fname], stdout=_subprocess.PIPE)
    proc2 = _subprocess.Popen(['tail', '-n', str(n)], stdin=proc1.stdout,
                             stdout=_subprocess.PIPE)
    stdout, stderr = proc2.communicate()
    return stdout

def tail(fname, n=1):
    """
    Run tail on a file.

    :param fname: name of file
    :param n: number of lines

    :type fname: str
    :type n: int
    """
    stub, ext = _os.path.splitext(fname)
    if ext == '.gz':
        return ztail(fname, n)
    else:
        return _subprocess.check_output(['tail', '-n', str(n), fname])

def zgrep(fname, regex):
    """
    Run grep on a gzipped file.

    :param fname: name of file
    :param regex: pattern to match

    :type fname: str
    :type regex: str
    """
    proc1 = _subprocess.Popen(['zcat', fname], stdout=_subprocess.PIPE)
    proc2 = _subprocess.Popen(['grep', regex], stdin=proc1.stdout,
                             stdout=_subprocess.PIPE)
    stdout, stderr = proc2.communicate()
    return stdout

def grep(fname, regex):
    """
    Run grep on a file.

    :param fname: name of file
    :param regex: pattern to match

    :type fname: str
    :type regex: str
    """
    stub, ext = _os.path.splitext(fname)
    if ext == '.gz':
        return zgrep(fname, regex)
    else:
        return _subprocess.check_output(['grep', regex, fname])

def parameter(fname, key):
    """
    Get parameter from keypoint file using grep.

    :param fname: name of file
    :param key: key to match

    :type fname: str
    :type key: str
    """
    assert key in langmuir.parameters._valid_keys
    assert _os.path.exists(fname)
    value = grep(fname, key)
    if value:
            value = eval(value.strip().split('=')[-1].strip())
    else:
        return key, None
    return key, value

def evaluate(obj):
    """
    Wrapper around eval.

    :param obj: arbitrary python object or string to evaluate
    :type obj: str, object
    """
    try:
        return eval(str(obj))
    except (NameError, SyntaxError, TypeError, AttributeError):
        return str(obj)

def timestamp(fname):
    """
    Get the last time a file was modified.

    :param fname: name of file
    :type fname: str
    """
    time = _os.path.getmtime(fname)
    return _datetime.datetime.fromtimestamp(time)

def zhandle(name, mode):
    """
    Get a file handle, checking to see if the file is gzipped first.

    :param name: filename
    :param mode: open mode

    :type name: str
    :type mode: str
    """
    stub, ext = _os.path.splitext(name)
    if ext == '.gz':
        return _gzip.open(name, mode)
    return open(name, mode)

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
    work = _os.getcwd()
    if not name is None:
        handle = open(name, 'w')
    else:
        handle = _sys.stdout

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
        print >> handle, "#    sed -i 's/\(voltage\.right\s*=\s*\).*$/\\1[-1 * v for v in range(0, 201, 10)]/' sim.inp"
        print >> handle, ''
        print >> handle, "#    sed -i 's/\(max\.threads\s*=\s*\).*$/\\14/' sim.inp"
        print >> handle, ''
        print >> handle, '#    ./submit'
    else:
        print >> handle, command
    print >> handle, ''
    print >> handle, '}'
    print >> handle, ''
    for i, sim in enumerate(paths):
        print >> handle, r'cd %s' % _os.path.relpath(sim, work)
        print >> handle, r'custom_command "%s%d"' % (stub, i)
        print >> handle, r'cd %s' % _os.path.relpath(work, sim)
        print >> handle, r''
    if not name is None:
        handle.close()
        _os.system('chmod +x %s' % name)
