# -*- coding: utf-8 -*-
"""
@author: adam
"""
import langmuir as lm
import glob
import os
import re

def find(work, r=False, single=True, absolute=True, stub='*', ext=None,
         exclude_dirs=False, exclude_files=False, sort_by=lm.regex.numbers,
         at_least_one=False, follow_links=False):
    """
    A method for searching for files and directories using patterns.

    single        : do not return a list
    recursive     : perform the search recursivly
    absolute      : return absolute paths
    stub          : the wildcard-able search pattern (`*` is the wildcard)
    exclude_dirs  : do not include directories in the search
    exclude_files : do not include files in the search
    sort_by       : a function (applied to paths) used to sort the results
    at_least_one  : make sure at least one thing was found
    follow_links  : do not follow symbolic links
    """
    work = os.path.expanduser(work)

    search = stub
    if not ext == None: search += '.%s' % ext.rstrip('.')

    result  = []

    if r:
        for root, dirs, files in os.walk(work, followlinks=follow_links):
            match = glob.glob(os.path.join(root,search))
            result.extend([os.path.abspath(i) for i in match])
    else:
        match = glob.glob(os.path.join(work,search))
        result.extend([os.path.abspath(i) for i in match])

    if exclude_files:
        result = [i for i in result if not os.path.isfile(i)]

    if exclude_dirs:
        result = [i for i in result if not os.path.isdir(i)]

    if not absolute:
        result = [os.path.relpath(i,work) for i in result]

    if not sort_by == None:
        result.sort(key=sort_by)

    if single:
        if not len(result) == 1:
            if len(result) == 0:
                raise RuntimeError('can not find %s in directory:\n\t%s'
                    % (search,work))
            raise RuntimeError('found multiple %s in directory:\n\t%s\n\t\t%s'
                %(search,work,'\n\t\t'.join([os.path.relpath(i,work)
                                                for i in result])))
        return result[0]

    if at_least_one:
        if len(result) == 0:
            raise RuntimeError('can not find %s in directory:\n\t%s'
                % (search, work))

    return result

def systems(work, **kwargs):
    """
    Search for directories that contain "runs".
    """
    kwargs.update(r=True)
    work  = os.path.expanduser(work)
    runs  = lm.find.runs(work, **kwargs)
    found = []
    for r in runs:
        dirname = os.path.dirname(r)
        if not dirname in found and not os.path.samefile(work, dirname):
            found.append(dirname)
    return found

def sims(work, **kwargs):
    """
    Search for directories that contain "parts".
    """
    kwargs.update(r=True)
    work  = os.path.expanduser(work)
    parts = lm.find.parts(work, **kwargs)
    found = []
    for p in parts:
        dirname = os.path.dirname(p)
        if not dirname in found and not os.path.samefile(work, dirname):
            found.append(dirname)
    return found

def runs(*args, **kwargs):
    _kwargs = dict(stub='run*', exclude_files=True, single=False)
    _kwargs.update(**kwargs)
    return find(*args, **_kwargs)

def run(*args, **kwargs):
    _kwargs = dict(stub='run*', exclude_files=True, single=True)
    _kwargs.update(**kwargs)
    return find(*args, **_kwargs)

def parts(*args, **kwargs):
    _kwargs = dict(stub='part*', exclude_files=True, single=False)
    _kwargs.update(**kwargs)
    return find(*args, **_kwargs)

def part(*args, **kwargs):
    _kwargs = dict(stub='part*', exclude_files=True, single=True)
    _kwargs.update(**kwargs)
    return find(*args, **_kwargs)

def inps(*args, **kwargs):
    _kwargs = dict(ext='inp*', exclude_dirs=True, single=False)
    _kwargs.update(**kwargs)
    return find(*args, **_kwargs)

def inp(*args, **kwargs):
    _kwargs = dict(ext='inp*', exclude_dirs=True, single=True)
    _kwargs.update(**kwargs)
    return find(*args, **_kwargs)

def chks(*args, **kwargs):
    _kwargs = dict(ext='chk*', exclude_dirs=True, single=False)
    _kwargs.update(**kwargs)
    return find(*args, **_kwargs)

def chk(*args, **kwargs):
    _kwargs = dict(ext='chk*', exclude_dirs=True, single=True)
    _kwargs.update(**kwargs)
    return find(*args, **_kwargs)

def parms(*args, **kwargs):
    _kwargs = dict(ext='parm*', exclude_dirs=True, single=False)
    _kwargs.update(**kwargs)
    return find(*args, **_kwargs)

def parm(*args, **kwargs):
    _kwargs = dict(ext='parm*', exclude_dirs=True, single=True)
    _kwargs.update(**kwargs)
    return find(*args, **_kwargs)

def dats(*args, **kwargs):
    _kwargs = dict(ext='dat*', exclude_dirs=True, single=False)
    _kwargs.update(**kwargs)
    return find(*args, **_kwargs)

def dat(*args, **kwargs):
    _kwargs = dict(ext='dat*', exclude_dirs=True, single=True)
    _kwargs.update(**kwargs)
    return find(*args, **_kwargs)

def pkls(*args, **kwargs):
    _kwargs = dict(ext='pkl*', exclude_dirs=True, single=False)
    _kwargs.update(**kwargs)
    return find(*args, **_kwargs)

def pkl(*args, **kwargs):
    _kwargs = dict(ext='pkl*', exclude_dirs=True, single=True)
    _kwargs.update(**kwargs)
    return find(*args, **_kwargs)

def pngs(*args, **kwargs):
    _kwargs = dict(ext='png*', exclude_dirs=True, single=False)
    _kwargs.update(**kwargs)
    return find(*args, **_kwargs)

def png(*args, **kwargs):
    _kwargs = dict(ext='png*', exclude_dirs=True, single=True)
    _kwargs.update(**kwargs)
    return find(*args, **_kwargs)

def slice_path(path, regex):
    """
    Return dirname of path where the regex matches
    """
    if isinstance(regex, str):
        regex = re.compile(regex)
    path = os.path.abspath(os.path.expanduser(path))
    assert len(regex.findall(path)) == 1
    path = path.split(os.sep)
    for i, item in enumerate(path):
        if not regex.match(item) is None:
            assert i + 1 <= len(path)
            return os.path.join(os.sep, *path[:i + 1])
            break
    raise RuntimeError('can not slice path on run')

def slice_part(path, regex='part'):
    """
    Return dirname of path where run directory is.
    """
    return slice_path(path, regex)


def slice_sim(path, regex='sim'):
    """
    Return dirname of path where run directory is.
    """
    return slice_path(path, regex)


def slice_run(path, regex='run'):
    """
    Return dirname of path where run directory is.
    """
    return slice_path(path, regex)