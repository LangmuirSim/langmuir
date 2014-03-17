# -*- coding: utf-8 -*-
"""
.. note::
    Utility functions.

.. moduleauthor:: Adam Gagorik <adam.gagorik@gmail.com>
"""
from subprocess import check_output, CalledProcessError
import collections
import textwrap
import StringIO
import pickle
import gzip
import sys
import os
import re

try:
    from scipy.misc import comb
except ImportError:
    def comb(N, k, exact=False):
        """
        Combinations: N choose k

        :param N: number of objects
        :param k: number of objects to choose

        :type N: int
        :type k: int

        :return: combinations
        :rtype: :py:obj:`int`

        >>> print comb(10, 2)
        45

        .. seealso:: :py:func:`scipy.misc.comb`
        """
        bc = [1 for i in range(0,k+1)]
        for j in range(1, N - k + 1):
            for i in range(1, k + 1):
                bc[i] += bc[i - 1]
        return bc[k]

def zhandle(handle, mode='rb'):
    """
    Open a file using gzip if necessary.

    :param handle: filename
    :type handle: str

    :return: file handle
    :rtype: :py:obj:`file`

    >>> handle = zhandle('out.dat.gz', 'rb')
    """
    try:
        if handle.endswith('.gz'):
            handle = gzip.open(handle, mode)
        else:
            handle = open(handle, mode)
    except AttributeError:
        pass
    return handle

def tail(handle, n=1, mode='python'):
    """
    Return last n lines of a file.  The python version is much faster than
    a system call.  The python version works on gzipped files.

    :param handle: filename
    :param n: lines
    :param mode: python or subprocess

    :type n: int
    :type mode: str

    :return: list of lines
    :rtype: :py:obj:`list` of :py:obj:`str`

    >>> lines = tail('out.dat', 1)
    """
    if mode == 'python':
        lines = [l.rstrip() for l in collections.deque(
            zhandle(handle, 'rb'), n)]
    else:
        if not isinstance(handle, str):
            try:
                handle = handle.name
            except AttributeError:
                pass

        command = 'tail -n {n:d} {handle}'.format(n=n, handle=handle)

        try:
            lines = check_output(command.strip().split())
        except (CalledProcessError, OSError):
            return tail(handle, n, mode='python')

    if lines:
        if len(lines) == 1:
            return lines[0]
        return lines

    return None

def splitext(handle):
    """
    Extract stub and extension from a file object or string.

    :param handle: filename

    :return: stub and ext
    :rtype: :py:obj:`str`, :py:obj:`str`

    >>> stub, ext = splitext('out.dat')
    >>> print stub, ext
    ('out', '.dat')
    """
    try:
        return os.path.splitext(handle)
    except AttributeError:
        try:
            return os.path.splitext(handle.name)
        except AttributeError:
            return '', ''

def load_pkl(handle, max_objs=256):
    """
    Load max objs from a pkl file.

    :param handle: filename
    :param max_objs: max number of objects to load

    :type handle: str
    :type max_objs: int

    :return: list of data
    :rtype: :py:obj:`list`

    >>> data = load_pkl('combined.pkl')
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

    :param pkls: list of filenames
    :type pkls: list of str

    :return: nested list of loaded data
    :rtype: :py:obj:`list`

    >>> pkls = load_pkls(['a.pkl', 'b.pkl'])
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

    :param obj: python object
    :type obj: object

    :return: the file handle
    :rtype: :py:obj:`file`

    >>> handle = save_pkl(1, 'test.pkl')
    >>> handle = save_pkl(2, handle)
    """
    handle = zhandle(handle, 'wb')
    pickle.dump(obj, handle, pickle.HIGHEST_PROTOCOL)
    return handle

def format_string(s, **kwargs):
    """
    Format a string using :py:func:`str.format` and process with regex.
    kwargs are passed to the format function.

    The following regex are used:

    ======= =========
    *match* *replace*
    ======= =========
    '^_'    ''
    '_$'    ''
    '_\.'   '.'
    ======= =========

    :param s: string to process
    :type s: str

    :return: the formatted string
    :rtype: :py:obj:`str`

    >>> print format_string('_{name}.{ext}', ext='dat', name='adam')
    adam.dat
    >>> print format_string('adam_.{ext}', ext='dat')
    adam.dat
    """
    return re.sub('_\.', '.', s.format(**kwargs).strip('-_'))

def format_output(s='{stub}_{name}.{ext}', stub='', name='out', ext='dat',
    **kwargs):
    """
    Format an output file name and then process with regex.  If stub is empty,
    the leading underscore is striped.

    :param stub: a unique string to identify the file
    :param name: a common string to identify the file
    :param ext: the filename extention without a '.'

    :type stub: str
    :type name: str
    :type ext: str

    :return: formatted filename
    :rtype: :py:obj:`str`

    >>> print format_output(stub='systemA', name='fft', ext='txt')
    systemA_fft.txt

    .. seealso:: :py:func:`format_string`
    """
    ext = ext.lstrip('.')
    _kwargs = dict(stub=stub, name=name, ext=ext)
    _kwargs.update(**kwargs)
    return format_string(s, **_kwargs)

def compare_dicts(dict1, dict2):
    """
    Compare two dictionaries.

    :param dict1: dict like object
    :param dict2: dict like object

    :type dict1: dict
    :type dict2: dict

    :return: a results dictionary with the output of set operations
    :rtype: :py:class:`collections.OrderedDict`

    >>> results = compare_dicts({'A' : 1}, {'A' : 2, 'B' : 3})
    >>> for key, value in results.iteritems():
    ...     print key, value
    size1 1      # size of dict1
    size2 2      # size of dict2
    union 2      # union of keys        (A, B)
    inter 1      # intersection of keys (A)
    valid False  # are they the same?
    A (1, 2)     # keys that are different and and their values
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

    :param list1: list like object
    :param list2: list like object

    :type list1: list
    :type list2: list

    :return: a results dictionary with the output of set operations
    :rtype: :py:class:`collections.OrderedDict`

    >>> results = compare_lists([1], [1, 2])
    >>> for key, value in results.iteritems():
    ...     print key, value
    size1 1      # size of list1
    size2 2      # size of list2
    union 2      # union of items        (1, 2)
    inter 1      # intersection of items (1)
    valid False  # are they the same?
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

def command_script(paths, name=None, cwd=None, batch='run.batch', batch_dir=r'~/templates', inp='sim.inp', comment=True,
                   stub='{relpath}', stub_func=None, **kwargs):
    """
    Create a handy script.

    :param paths: list of paths
    :param name: output file name
    :param cwd: working directory
    :param batch: batch file name
    :param batch_dir: batch file directory
    :param inp: input file name
    :param comment: comment out custom command?
    :param stub: job stub string
    :param stub_func: post-process stub

    :type paths: list
    :type name: str
    :type cwd: str
    :type batch: str
    :type batch_dir: str
    :type inp: str
    :type comment: str
    :type stub: str
    :type stub_func: func
    """
    if cwd is None:
        cwd = os.getcwd()

    script = """
    #!/bin/bash

    FILE={batch_dir}/{batch}

    custom_command()
    {{
        echo ""
        echo "--------------------------------------------------------------------------------"
        echo `pwd`
        echo "$1"
        echo "--------------------------------------------------------------------------------"

        # submitLangmuir $1 {inp}

        # cp $FILE .

        # qsub -N $1 {batch}

        # langmuir {inp}

        # grep "max.threads" {inp}

        # sed -i 's/\(grid\.z\s*=\s*\).*$/\\11/' {inp}

        # lscan --real 10000000 --print 1000 --fmt '%+.1f' --stub '{{working}}_{{value}}' --mode gen --job "$1_{{i}}" {inp}

        # sed -i 's/# \.\/submit/.\/submit/' submit

        # ./submit
    }}

    {command}
    """

    sub_command = StringIO.StringIO()
    for i, path in enumerate(paths):

        relpath = os.path.relpath(path, cwd)
        dirname, basename = os.path.dirname(relpath), os.path.basename(relpath)

        _kwargs = dict(i=i, path=path, relpath=relpath, dirname=dirname, basename=basename)
        _kwargs.update(**kwargs)

        sim = stub.format(**_kwargs)
        try:
            sim = stub_func(sim)
        except TypeError:
            pass

        print >> sub_command, r'cd {path}'.format(path=os.path.relpath(path, cwd))
        if comment:
            print >> sub_command, r'# custom_command "{sim}"'.format(sim=sim)
        else:
            print >> sub_command, r'# custom_command "{sim}"'.format(sim=sim)
        print >> sub_command, r'cd {path}'.format(path=os.path.relpath(cwd, path))
        print >> sub_command, r''

    sub_command = sub_command.getvalue().strip()

    script = textwrap.dedent(script).strip('\n').format(command=sub_command, batch=batch, batch_dir=batch_dir, inp=inp)

    if name is None:
        print script
    else:
        with open(name, 'w') as handle:
            handle.write(script)
        os.system('chmod +x {name}'.format(name=name))