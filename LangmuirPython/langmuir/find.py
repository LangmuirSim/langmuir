# -*- coding: utf-8 -*-
from langmuir.regex import numbers as _sort_by_numbers
from langmuir.regex import part as _part_id
from langmuir.regex import run as _run_id
from langmuir.common import parameter, tail, zhandle
import fnmatch as _fnmatch
import numpy as _np
import glob as _glob
import os as _os
import re as _re

def find(work, single=True, recursive=True, absolute=True, stub='*',
         ext=None, exclude_dirs=False, exclude_files=False,
         sort_by=_sort_by_numbers, at_least_one=False,
         follow_links=False):
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

    search = stub
    if not ext == None: search += '.%s' % ext.rstrip('.')

    result  = []

    if recursive:
        for root, dirs, files in _os.walk(work, followlinks=follow_links):
            match = _glob.glob(_os.path.join(root,search))
            result.extend([_os.path.abspath(i) for i in match])
    else:
        match = _glob.glob(_os.path.join(work,search))
        result.extend([_os.path.abspath(i) for i in match])

    if exclude_files:
        result = [i for i in result if not _os.path.isfile(i)]

    if exclude_dirs:
        result = [i for i in result if not _os.path.isdir(i)]

    if not absolute:
        result = [_os.path.relpath(i,work) for i in result]

    if not sort_by == None:
        result.sort(key=sort_by)

    if single:
        if not len(result) == 1:
            if len(result) == 0:
                raise RuntimeError('can not find %s in directory:\n\t%s'
                    % (search,work))
            raise RuntimeError('found multiple %s in directory:\n\t%s\n\t\t%s'
                %(search,work,'\n\t\t'.join([_os.path.relpath(i,work)
                                                for i in result])))
        return result[0]

    if at_least_one:
        if len(result) == 0:
            raise RuntimeError('can not find %s in directory:\n\t%s'
                % (search, work))

    return result

def systems(work, stub=None, recursive=False, absolute=True,
            at_least_one=False, follow_links=False,
            sort_by=_sort_by_numbers):

    if recursive:
        paths = runs(work, recursive=True, absolute=True,
                     follow_links=follow_links)
        temp = []
        for path in paths:
            path = _os.path.dirname(path)
            if not path in temp:
                temp.append(path)
        paths = temp
    else:
        work, paths, files = _os.walk(work).next()
        temp = []
        for path in paths:
            if runs(path, recursive=False, absolute=True,
                    follow_links=follow_links):
                temp.append(path)
        paths = temp

    if not stub is None:
        paths = _fnmatch.filter(paths, stub)

    if absolute:
        paths = [_os.path.abspath(path) for path in paths]
    else:
        paths = [_os.path.relpath(path, work) for path in paths]

    if at_least_one:
        if len(paths) == 0:
            raise RuntimeError('can not find system in directory:\n\t%s'
                % (work))

    if not sort_by == None:
        paths.sort(key=sort_by)
    else:
        paths.sort()

    return paths

def system(work, recursive=False, absolute=True, at_least_one=False,
           follow_links=False, sort_by=_sort_by_numbers):
    paths = systems(work, stub=None, recursive=recursive, absolute=absolute,
                    at_least_one=at_least_one, follow_links=follow_links,
                    sort_by=sort_by)
    if not len(paths) == 1:
        if len(paths) == 0:
            raise RuntimeError('can not find system in directory:\n\t%s'
                % (work))
        raise RuntimeError(
            'found multiple systems in directory:\n\t%s\n\t\t%s'
            %(work,'\n\t\t'.join([_os.path.relpath(i,work)
                                            for i in paths])))
    return paths[0]

def runs(work, recursive=False, absolute=True, at_least_one=False,
         follow_links=False):
    result = find(work, single=False, recursive=recursive, absolute=absolute,
         stub='run*', ext=None, exclude_dirs=False, exclude_files=True,
         at_least_one=at_least_one, follow_links=follow_links)
    return result

def run(work, stub='run*', recursive=False, absolute=True,
        at_least_one=False, follow_links=False):
    return find(work, single=True, recursive=recursive, absolute=absolute,
         stub=stub, ext=None, exclude_dirs=False, exclude_files=True,
         at_least_one=at_least_one, follow_links=follow_links)

def sims(work, stub='sim*', recursive=False, absolute=True,
         at_least_one=False, follow_links=False):
    result = find(work, single=False, recursive=recursive, absolute=absolute,
         stub=stub, ext=None, exclude_dirs=False, exclude_files=True,
         at_least_one=at_least_one, follow_links=follow_links)
    return result

def sim(work, stub='sim*', recursive=False, absolute=True,
        at_least_one=False, follow_links=False):
    return find(work, single=True, recursive=recursive, absolute=absolute,
         stub=stub, ext=None, exclude_dirs=False, exclude_files=True,
         at_least_one=at_least_one, follow_links=follow_links)

def parts(work, recursive=False, absolute=True, at_least_one=False,
          follow_links=False):
    result = find(work, single=False, recursive=recursive, absolute=absolute,
         stub='part*', ext=None, exclude_dirs=False, exclude_files=True,
         at_least_one=at_least_one, follow_links=follow_links)
    return result

def part(work, stub='part*', recursive=False, absolute=True,
         at_least_one=False, follow_links=False):
    return find(work, single=True, recursive=recursive, absolute=absolute,
         stub=stub, ext=None, exclude_dirs=False, exclude_files=True,
         at_least_one=at_least_one, follow_links=follow_links)

def inp(work, stub='*', recursive=False, absolute=True,
        at_least_one=False, follow_links=False):
    return find(work, single=True, recursive=recursive, absolute=absolute,
         stub=stub, ext='inp*', exclude_dirs=True, exclude_files=False,
         at_least_one=at_least_one, follow_links=follow_links)

def inps(work, stub='*', recursive=False, absolute=True,
         at_least_one=False, follow_links=False):
    return find(work, single=False, recursive=recursive, absolute=absolute,
         stub=stub, ext='inp*', exclude_dirs=True, exclude_files=False,
         at_least_one=at_least_one, follow_links=follow_links)

def chk(work, stub='*', recursive=False, absolute=True,
        at_least_one=False, follow_links=False):
    return find(work, single=True, recursive=recursive, absolute=absolute,
         stub=stub, ext='chk*', exclude_dirs=True, exclude_files=False,
         at_least_one=at_least_one, follow_links=follow_links)

def chks(work, stub='*', recursive=False, absolute=True,
         at_least_one=False, follow_links=False):
    return find(work, single=False, recursive=recursive, absolute=absolute,
         stub=stub, ext='chk*', exclude_dirs=True, exclude_files=False,
         at_least_one=at_least_one, follow_links=follow_links)

def dat(work, stub='*', recursive=False, absolute=True,
        at_least_one=False, follow_links=False):
    return find(work, single=True, recursive=recursive, absolute=absolute,
         stub=stub, ext='dat*', exclude_dirs=True, exclude_files=False,
         at_least_one=at_least_one, follow_links=follow_links)

def dats(work, stub='*', recursive=False, absolute=True,
         at_least_one=False, follow_links=False):
    return find(work, single=False, recursive=recursive, absolute=absolute,
         stub=stub, ext='dat*', exclude_dirs=True, exclude_files=False,
         at_least_one=at_least_one, follow_links=follow_links)

def pkl(work, stub='*', recursive=False, absolute=True,
        at_least_one=False, follow_links=False):
    return find(work, single=True, recursive=recursive, absolute=absolute,
         stub=stub, ext='pkl*', exclude_dirs=True, exclude_files=False,
         at_least_one=at_least_one)

def pkls(work, stub='*', recursive=False, absolute=True,
         at_least_one=False, follow_links=False):
    return find(work, single=False, recursive=recursive, absolute=absolute,
         stub=stub, ext='pkl*', exclude_dirs=True, exclude_files=False,
         at_least_one=at_least_one, follow_links=follow_links)

def npy(work, stub='*', recursive=False, absolute=True,
        at_least_one=False, follow_links=False):
    return find(work, single=True, recursive=recursive, absolute=absolute,
         stub=stub, ext='npy*', exclude_dirs=True, exclude_files=False,
         at_least_one=at_least_one, follow_links=follow_links)

def npys(work, stub='*', recursive=False, absolute=True,
         at_least_one=False, follow_links=False):
    return find(work, single=False, recursive=recursive, absolute=absolute,
         stub=stub, ext='npy*', exclude_dirs=True, exclude_files=False,
         at_least_one=at_least_one, follow_links=follow_links)

def npz(work, stub='*', recursive=False, absolute=True,
        at_least_one=False, follow_links=False):
    return find(work, single=True, recursive=recursive, absolute=absolute,
         stub=stub, ext='npz*', exclude_dirs=True, exclude_files=False,
         at_least_one=at_least_one, follow_links=follow_links)

def npzs(work, stub='*', recursive=False, absolute=True,
         at_least_one=False, follow_links=False):
    return find(work, single=False, recursive=recursive, absolute=absolute,
         stub=stub, ext='npz*', exclude_dirs=True, exclude_files=False,
         at_least_one=at_least_one, follow_links=follow_links)

def png(work, stub='*', recursive=False, absolute=True,
        at_least_one=False, follow_links=False):
    return find(work, single=True, recursive=recursive, absolute=absolute,
         stub=stub, ext='png*', exclude_dirs=True, exclude_files=False,
         at_least_one=at_least_one, follow_links=follow_links)

def pngs(work, stub='*', recursive=False, absolute=True,
         at_least_one=False, follow_links=False):
    return find(work, single=False, recursive=recursive, absolute=absolute,
         stub=stub, ext='png*', exclude_dirs=True, exclude_files=False,
         at_least_one=at_least_one, follow_links=follow_links)

def pdf(work, stub='*', recursive=False, absolute=True,
        at_least_one=False, follow_links=False):
    return find(work, single=True, recursive=recursive, absolute=absolute,
         stub=stub, ext='pdf*', exclude_dirs=True, exclude_files=False,
         at_least_one=at_least_one, follow_links=follow_links)

def pdfs(work, stub='*', recursive=False, absolute=True,
         at_least_one=False, follow_links=False):
    return find(work, single=False, recursive=recursive, absolute=absolute,
         stub=stub, ext='pdf*', exclude_dirs=True, exclude_files=False,
         at_least_one=at_least_one, follow_links=follow_links)

def jpg(work, stub='*', recursive=False, absolute=True,
        at_least_one=False, follow_links=False):
    return find(work, single=True, recursive=recursive, absolute=absolute,
         stub=stub, ext='jpg*', exclude_dirs=True, exclude_files=False,
         at_least_one=at_least_one, follow_links=follow_links)

def jpgs(work, stub='*', recursive=False, absolute=True,
         at_least_one=False, follow_links=False):
    return find(work, single=False, recursive=recursive, absolute=absolute,
         stub=stub, ext='jpg*', exclude_dirs=True, exclude_files=False,
         at_least_one=at_least_one, follow_links=follow_links)

def jpeg(work, stub='*', recursive=False, absolute=True,
         at_least_one=False, follow_links=False):
    return find(work, single=True, recursive=recursive, absolute=absolute,
         stub=stub, ext='jpeg*', exclude_dirs=True, exclude_files=False,
         at_least_one=at_least_one, follow_links=follow_links)

def jpegs(work, stub='*', recursive=False, absolute=True,
          at_least_one=False, follow_links=False):
    return find(work, single=False, recursive=recursive, absolute=absolute,
         stub=stub, ext='jpeg*', exclude_dirs=True, exclude_files=False,
         at_least_one=at_least_one, follow_links=follow_links)

def parm(work, stub='*', recursive=False, absolute=True,
         at_least_one=False, follow_links=False):
    return find(work, single=True, recursive=recursive, absolute=absolute,
         stub=stub, ext='parm*', exclude_dirs=True, exclude_files=False,
         at_least_one=at_least_one, follow_links=follow_links)

def parms(work, stub='*', recursive=False, absolute=True,
          at_least_one=False, follow_links=False):
    return find(work, single=False, recursive=recursive, absolute=absolute,
         stub=stub, ext='parm*', exclude_dirs=True, exclude_files=False,
         at_least_one=at_least_one, follow_links=follow_links)

def coulomb(work, stub='*', recursive=False, absolute=True,
            at_least_one=False, follow_links=False):
    return find(work, single=True, recursive=recursive, absolute=absolute,
         stub=stub, ext='coulomb*', exclude_dirs=True, exclude_files=False,
         at_least_one=at_least_one, follow_links=follow_links)

def coulombs(work, stub='*', recursive=False, absolute=True,
             at_least_one=False, follow_links=False):
    return find(work, single=False, recursive=recursive, absolute=absolute,
         stub=stub, ext='coulomb*', exclude_dirs=True, exclude_files=False,
         at_least_one=at_least_one, follow_links=follow_links)

def time(work, stub='*', recursive=False, absolute=True,
            at_least_one=False, follow_links=False):
    return find(work, single=True, recursive=recursive, absolute=absolute,
         stub=stub, ext='time*', exclude_dirs=True, exclude_files=False,
         at_least_one=at_least_one, follow_links=follow_links)

def times(work, stub='*', recursive=False, absolute=True,
             at_least_one=False, follow_links=False):
    return find(work, single=False, recursive=recursive, absolute=absolute,
         stub=stub, ext='time*', exclude_dirs=True, exclude_files=False,
         at_least_one=at_least_one, follow_links=follow_links)

def files(work, stub='*', recursive=False, absolute=True,
          at_least_one=False, follow_links=False):
    return find(work, single=False, recursive=recursive, absolute=absolute,
         stub=stub, ext=None, exclude_dirs=True, exclude_files=False,
         at_least_one=at_least_one, follow_links=follow_links)

def dirs(work, stub='*', recursive=False, absolute=True,
         at_least_one=False, follow_links=False):
    return find(work, single=False, recursive=recursive, absolute=absolute,
         stub=stub, ext=None, exclude_dirs=False, exclude_files=True,
         at_least_one=at_least_one, follow_links=follow_links)

"""
Background about how simulation directories should be structured:

    example: run/voltage.right_(value)/part.(number)/(output files)

There is a main directory for the project.

    example: run

In this directory are many simulation directories, for example, one for
each value of the working variable scanned.

    example:
        run/voltage.right_+0.2
        run/voltage.right_+0.4
        run/voltage.right_+0.6
        run/voltage.right_+0.8
        run/voltage.right_+1.0

In each simulation directory is a directory for each part.  There are parts
because Languir has the ability to use checkpoint files.  Even if there is
only one part for a simulation, it should still be placed in a part.0
directory.

    example:
        run/voltage.right_+0.2/part.0
        run/voltage.right_+0.2/part.1
        run/voltage.right_+0.2/part.2
        ...

In each part directory are the input and output files from a single Langmuir
simulation.  (The part directories are the working directories for Langmuir
simulations)

    example:
        run/voltage.right_+0.2/part.0/out.dat.gz
        run/voltage.right_+0.2/part.0/out.chk.gz
        ...

Note: In general, this python module can load/find files that are gzipped,
or not.
"""

class Part:
    """
    Looks for simulation output files in a part directory

        example path: run/sim/part.0

    Files searched for include:
        stub.dat
        stub.chk
        stub.parm
        stub.time

    The default stub from Langmuir is stub=out, so files are name out.dat, etc.

    work: the path of the part directory
    stub: the output file stub
    """
    def __init__(self, work, stub='*'):
        self.work = work
        self.sim  = None

        try:
            self.dat = dat(self.work, stub=stub)
        except:
            self.dat = None

        try:
            self.chk = chk(self.work, stub=stub)
        except:
            self.chk = None

        try:
            self.time = time(self.work, stub=stub)
        except:
            self.time = None

        try:
            self.parm = parm(self.work, stub=stub)
        except:
            self.parm = None

        self.step    = None
        self.real    = None
        self.length  = None
        self.lunit   = ''
        self.percent = None
        self.id      = _part_id(self.work)

    def get_step(self):
        if self.chk:
            self.step = parameter(self.chk, 'current.step')[-1]
        elif self.dat:
            self.step = int(tail(self.dat).strip().split()[0])
        else:
            self.step = None

    def get_real(self):
        if self.chk:
            self.real = parameter(self.chk, 'iterations.real')[-1]
        else:
            self.real = None

    def get_time(self):
        if self.time:
            handle = zhandle(self.time, 'r')
            self.length = _np.genfromtxt(handle, names=True)
            L = self.length['msecs']
            self.lunit = 'ms'
            if L >= 1000:
                L = self.length['secs']
                self.lunit = 'sec'
            if L >= 60:
                L = self.length['min']
                self.lunit = 'min'
            if L >= 60:
                L = self.length['hours']
                self.lunit = 'hr'
            if L >= 24:
                L = self.length['days']
                self.lunit = 'day'
            self.length = L
        elif self.dat:
            self.length = int(tail(self.dat).strip().split()[-1])
            self.lunit = 'ms'
            if self.length >= 1000:
                self.length /= 1000.0
                self.lunit = 'sec'
            if self.length >= 60:
                self.length /= 60.0
                self.lunit = 'min'
            if self.length >= 60:
                self.length /= 60.0
                self.lunit = 'hr'
            if self.length >= 24:
                self.length /= 24.0
                self.lunit = 'day'
        else:
            self.length = None

    def get_percent(self):
        if not self.real:
            self.get_real()
        if not self.step:
            self.get_step()
        if self.real and self.step:
            self.percent = float(self.step) / float(self.real) * 100
        else:
            self.percent = None

    def __str__(self):
        if self.sim:
            path = _os.path.relpath(self.work, self.sim.work)
        else:
            path = _os.path.relpath(self.work, _os.path.dirname(self.work))

        if self.dat:
            dat = True
        else:
            dat = False

        if self.chk:
            chk = True
        else:
            chk = False

        if self.percent and self.length:
            return '%s [id=%d chk=%s dat=%s step=%s, real=%s] %s%% %6.3f %s' % (
                path, self.id, chk, dat, self.step, self.real, self.percent,
                self.length, self.lunit)
        elif self.percent:
            return '%s [id=%d chk=%s dat=%s step=%s, real=%s] %s%%' % (path,
                self.id, chk, dat, self.step, self.real, self.percent)
        elif self.length:
            return '%s [id=%d chk=%s dat=%s step=%s, real=%s] %6.3f %s' % (path,
                self.id, chk, dat, self.step, self.real, self.length,
                self.lunit)
        else:
            return '%s [id=%d chk=%s dat=%s step=%s, real=%s]' % (path,
                self.id, chk, dat, self.step, self.real)

class Sim:
    """
    Looks for parts in a simulation directory

        example path: run/sim

    The directories searched for:
        run/sim/part.0
        run/sim/part.1
        run/sim/part.2
        ...

    work: the path of the part directory
    stub: the output file stub (passed to the Part constructor)
    """
    def __init__(self, work, stub='*'):
        self.work  = work
        self.parts = []
        paths = parts(self.work, at_least_one=False)
        for path in paths:
            part = Part(path, stub)
            part.sim = self
            self.parts.append(part)
        self.run = None
        try:
            self.value = eval(_os.path.basename(
                self.work).strip().split('_')[-1])
        except:
            self.value = None

    def __iter__(self):
        return self.parts.__iter__()

    def __getitem__(self, index):
        return self.parts[index]

    def __str__(self):
        if self.run:
            path = _os.path.relpath(self.work, self.run.work)
        else:
            path =_os.path.relpath(self.work, _os.path.dirname(self.work))
        return '%s [value=%s, parts=%d]' % (path, self.value,
            len(self.parts))

class Run:
    """
    Looks for simulations in a run directory

        example path: run

    The directories searched for:
        run/sim.0
        run/sim.1
        run/sim.2
        ...

    A more common example might be:
        run/voltage.right_+0.2
        run/voltage.right_+0.4
        run/voltage.right_+0.6
        ...

    Use sim_stub='voltage.right*' to search for these directories

    work    : the path of the part directory
    sim_stub: the simulation directory stub (example=voltage.right*)
    stub    : the output file stub (passed to the Part constructor)
    """
    def __init__(self, work, sim_stub='voltage*', stub='*'):
        self.work = work
        self.sims = []
        paths = sims(self.work, stub=sim_stub, at_least_one=False)
        for path in paths:
            sim = Sim(path, stub)
            sim.run = self
            self.sims.append(sim)
        self.system = None
        self.id = _run_id(self.work)

    def __iter__(self):
        return self.sims.__iter__()

    def __getitem__(self, index):
        return self.sims[index]

    def __str__(self):
        if self.system:
            path = _os.path.relpath(self.work, self.system.work)
        else:
            path = _os.path.relpath(self.work, _os.path.dirname(self.work))
        return '%s [id=%d, sims=%d]' % (path, self.id, len(self.sims))

class System:
    def __init__(self, work, sim_stub='voltage*', stub='*'):
        self.work = work
        self.runs = []
        self.name = _os.path.basename(self.work)
        paths = runs(self.work, at_least_one=False)
        for path in paths:
            run = Run(path, sim_stub, stub)
            run.system = self
            self.runs.append(run)

    def __iter__(self):
        return self.runs.__iter__()

    def __getitem__(self, index):
        return self.runs[index]

    def __str__(self):
        return '%s [runs=%d]' % (self.name, len(self.runs))


def slice_path(path, regex):
    """
    Return dirname of path where the regex matches
    """
    if isinstance(regex, str):
        regex = _re.compile(regex)
    path = _os.path.abspath(_os.path.expanduser(path))
    assert len(regex.findall(path)) == 1
    path = path.split(_os.sep)
    for i, item in enumerate(path):
        if not regex.match(item) is None:
            assert i + 1 <= len(path)
            return _os.path.join(_os.sep, *path[:i + 1])
            break
    raise RuntimeError('can not slice path on run')

def slice_part(path, regex='part'):
    """
    Return dirname of path where run directory is
    """
    return slice_path(path, regex)


def slice_sim(path, regex='sim'):
    """
    Return dirname of path where run directory is
    """
    return slice_path(path, regex)


def slice_run(path, regex='run'):
    """
    Return dirname of path where run directory is
    """
    return slice_path(path, regex)
