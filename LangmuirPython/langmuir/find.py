# -*- coding: utf-8 -*-
"""
.. note::
    Functions for performing file searching.

.. moduleauthor:: Adam Gagorik <adam.gagorik@gmail.com>
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

    :param work: path to search
    :param r: perform the search recursivly
    :param single: do not return a list
    :param absolute: return absolute paths
    :param stub: the wildcard-able search pattern (star is the wildcard)
    :param ext: the filename extension
    :param exclude_dirs: do not include directories in the search
    :param exclude_files: do not include files in the search
    :param sort_by: a function (applied to paths) used to sort the results
    :param at_least_one: make sure at least one thing was found
    :param follow_links: do not follow symbolic links

    :type work: str
    :type r: bool
    :type single: bool
    :type absolute: bool
    :type stub: str
    :type ext: str
    :type exclude_dirs: bool
    :type exclude_files: bool
    :type sort_by: func
    :type at_least_one: bool
    :type follow_links: bool

    >>> import os
    >>> work = os.getcwd()
    >>> pkls = find(work, ext='txt*') # find all txt files in work
    """
    work = os.path.expanduser(work)

    search = stub
    if not ext is None:
        search += '.%s' % ext.rstrip('.')

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

    if not sort_by is None:
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

    :param args: see :func:`runs`
    :param kwargs: see :func:`runs`

    >>> systems = lm.find.systems(r'/home/adam/simulations')
    >>> print '\\n'.join(systems)
    '/home/adam/simulations/systemA'
    '/home/adam/simulations/systemB'
    '/home/adam/simulations/systemC'
    '/home/adam/simulations/systemD'
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

    :param args: see :func:`parts`
    :param kwargs: see :func:`parts`

    >>> work = r'/home/adam/simulations/systemA/run.0'
    >>> sims = lm.find.sims(work, stub='voltage.right')
    >>> print '\\n'.join(systems)
    '/home/adam/simulations/systemA/run.0/voltage.right_+0.0'
    '/home/adam/simulations/systemA/run.0/voltage.right_+0.2'
    '/home/adam/simulations/systemA/run.0/voltage.right_+0.4'
    '/home/adam/simulations/systemA/run.0/voltage.right_+0.8'
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
    """
    Search for directories of the form run*.

    :param args: see :func:`find`
    :param kwargs: see :func:`find`

    >>> runs = lm.find.runs(r'/home/adam/simulations/systemA/')
    >>> print '\\n'.join(runs)
    '/home/adam/simulations/systemA/run.0/'
    '/home/adam/simulations/systemA/run.1/'
    '/home/adam/simulations/systemA/run.2/'
    '/home/adam/simulations/systemA/run.3/'
    """
    _kwargs = dict(stub='run*', exclude_files=True, single=False)
    _kwargs.update(**kwargs)
    return find(*args, **_kwargs)

def run(*args, **kwargs):
    """
    Search for a single directory of the form run*.

    :param args: see :func:`find`
    :param kwargs: see :func:`find`

    >>> run = lm.find.run(r'/home/adam/simulations/systemA')
    RuntimeError: found multiple run* in directory:
        /home/adam/simulations/systemA
            run.0
            run.1
            run.2
            run.3
    """
    _kwargs = dict(stub='run*', exclude_files=True, single=True)
    _kwargs.update(**kwargs)
    return find(*args, **_kwargs)

def parts(*args, **kwargs):
    """
    Search for directories of the form part*.

    :param args: see :func:`find`
    :param kwargs: see :func:`find`

    >>> prts = lm.find.parts(r'~/simulations/systemA/run.0/voltage.right_+0.0')
    >>> print '\\n'.join(prts)
    '/home/adam/simulations/systemA/run.0/voltage.right_+0.0/part.0/'
    '/home/adam/simulations/systemA/run.0/voltage.right_+0.0/part.1/'
    """
    _kwargs = dict(stub='part*', exclude_files=True, single=False)
    _kwargs.update(**kwargs)
    return find(*args, **_kwargs)

def part(*args, **kwargs):
    """
    Search for a single directory of the form part*.

    :param args: see :func:`find`
    :param kwargs: see :func:`find`

    >>> part = lm.find.parts(r'~/simulations/systemA/run.0/voltage.right_+0.0')
    RuntimeError: found multiple part* in directory:
        /home/adam/simulations/systemA/run.0/voltage.right_+0.0
            part.0
            part.1
    """
    _kwargs = dict(stub='part*', exclude_files=True, single=True)
    _kwargs.update(**kwargs)
    return find(*args, **_kwargs)

def inps(*args, **kwargs):
    """
    Search for files of the form *.inp*.

    :param args: see :func:`find`
    :param kwargs: see :func:`find`

    >>> inps = lm.find.inps('.')
    """
    _kwargs = dict(ext='inp*', exclude_dirs=True, single=False)
    _kwargs.update(**kwargs)
    return find(*args, **_kwargs)

def inp(*args, **kwargs):
    """
    Search for a single file of the form *.inp*.

    :param args: see :func:`find`
    :param kwargs: see :func:`find`

    >>> inp = lm.find.inp('.')
    """
    _kwargs = dict(ext='inp*', exclude_dirs=True, single=True)
    _kwargs.update(**kwargs)
    return find(*args, **_kwargs)

def chks(*args, **kwargs):
    """
    Search for files of the form *.chk*.

    :param args: see :func:`find`
    :param kwargs: see :func:`find`

    >>> chks = lm.find.chks('.')
    """
    _kwargs = dict(ext='chk*', exclude_dirs=True, single=False)
    _kwargs.update(**kwargs)
    return find(*args, **_kwargs)

def chk(*args, **kwargs):
    """
    Search for a single file of the form *.chk*.

    :param args: see :func:`find`
    :param kwargs: see :func:`find`

    >>> chk = lm.find.chk('.')
    """
    _kwargs = dict(ext='chk*', exclude_dirs=True, single=True)
    _kwargs.update(**kwargs)
    return find(*args, **_kwargs)

def parms(*args, **kwargs):
    """
    Search for files of the form *.parm*.

    :param args: see :func:`find`
    :param kwargs: see :func:`find`

    >>> parms = lm.find.parms('.')
    """
    _kwargs = dict(ext='parm*', exclude_dirs=True, single=False)
    _kwargs.update(**kwargs)
    return find(*args, **_kwargs)

def parm(*args, **kwargs):
    """
    Search for a single file of the form *.parm*.

    :param args: see :func:`find`
    :param kwargs: see :func:`find`

    >>> parm = lm.find.parm('.')
    """
    _kwargs = dict(ext='parm*', exclude_dirs=True, single=True)
    _kwargs.update(**kwargs)
    return find(*args, **_kwargs)

def dats(*args, **kwargs):
    """
    Search for files of the form *.dat*.

    :param args: see :func:`find`
    :param kwargs: see :func:`find`

    >>> dats = lm.find.dats('.')
    """
    _kwargs = dict(ext='dat*', exclude_dirs=True, single=False)
    _kwargs.update(**kwargs)
    return find(*args, **_kwargs)

def dat(*args, **kwargs):
    """
    Search for a single file of the form *.dat*.

    :param args: see :func:`find`
    :param kwargs: see :func:`find`

    >>> dat = lm.find.dat('.')
    """
    _kwargs = dict(ext='dat*', exclude_dirs=True, single=True)
    _kwargs.update(**kwargs)
    return find(*args, **_kwargs)

def txts(*args, **kwargs):
    """
    Search for files of the form *.txt*.

    :param args: see :func:`find`
    :param kwargs: see :func:`find`

    >>> txts = lm.find.txts('.')
    """
    _kwargs = dict(ext='txt*', exclude_dirs=True, single=False)
    _kwargs.update(**kwargs)
    return find(*args, **_kwargs)

def txt(*args, **kwargs):
    """
    Search for a single file of the form *.txt*.

    :param args: see :func:`find`
    :param kwargs: see :func:`find`

    >>> txt = lm.find.txt('.')
    """
    _kwargs = dict(ext='txt*', exclude_dirs=True, single=True)
    _kwargs.update(**kwargs)
    return find(*args, **_kwargs)

def pkls(*args, **kwargs):
    """
    Search for files of the form *.pkl*.

    :param args: see :func:`find`
    :param kwargs: see :func:`find`

    >>> pkls = lm.find.pkls('.')
    """
    _kwargs = dict(ext='pkl*', exclude_dirs=True, single=False)
    _kwargs.update(**kwargs)
    return find(*args, **_kwargs)

def pkl(*args, **kwargs):
    """
    Search for a single file of the form *.pkl*.

    :param args: see :func:`find`
    :param kwargs: see :func:`find`

    >>> pkl = lm.find.pkl('.')
    """
    _kwargs = dict(ext='pkl*', exclude_dirs=True, single=True)
    _kwargs.update(**kwargs)
    return find(*args, **_kwargs)

def pngs(*args, **kwargs):
    """
    Search for files of the form *.png*.

    :param args: see :func:`find`
    :param kwargs: see :func:`find`

    >>> pngs = lm.find.pngs('.')
    """
    _kwargs = dict(ext='png*', exclude_dirs=True, single=False)
    _kwargs.update(**kwargs)
    return find(*args, **_kwargs)

def png(*args, **kwargs):
    """
    Search for a single file of the form *.png*.

    :param args: see :func:`find`
    :param kwargs: see :func:`find`

    >>> png = lm.find.png('.')
    """
    _kwargs = dict(ext='png*', exclude_dirs=True, single=True)
    _kwargs.update(**kwargs)
    return find(*args, **_kwargs)

def slice_path(path, regex):
    """
    Return dirname of path where the regex matches.

    :param path: path to parse
    :param regex: regex to match

    :type path: str
    :type regex: str

    >>> print slice_path('r/home/adam/Desktop', 'adam')
    '/home/adam'
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
    raise RuntimeError('can not slice path on run')

def slice_part(path):
    """
    Return dirname of path where run directory is.

    :param path: path to parse
    :type path: str

    >>> print slice_path('r/system/run/sim/part')
    '/system/run/sim/part'
    """
    return slice_path(path, 'part')


def slice_sim(path):
    """
    Return dirname of path where sim directory is.

    :param path: path to parse
    :type path: str

    >>> print slice_path('r/system/run/sim/part')
    '/system/run/sim'
    """
    return slice_path(path, 'sim')


def slice_run(path):
    """
    Return dirname of path where run directory is.

    :param path: path to parse
    :type path: str

    >>> print slice_path('r/system/run/sim/part')
    '/system/run'
    """
    return slice_path(path, 'run')

def slice_system(path):
    """
    Return dirname of path where system directory is.

    :param path: path to parse
    :type path: str

    >>> print slice_path('r/system/run/sim/part')
    '/system'
    """
    return os.path.dirname(slice_run(path))

def extract_system(path):
    """
    Extract system name from path.

    :param path: path to parse
    :type path: str
    """
    return os.path.basename(slice_system(path))