# -*- coding: utf-8 -*-
"""
.. note::
    Functions for opening, editing, and saving Langmuir input files.

.. moduleauthor:: Adam Gagorik <adam.gagorik@gmail.com>
"""
import langmuir as lm
import collections
import StringIO
import re

try:
    import numpy as np
except ImportError:
    pass

try:
    import pandas as pd
except:
    ImportError

Parameter = collections.namedtuple('Parameter',
    ['key', 'pytype', 'default', 'units', 'fmt'])

parameters = [
    Parameter('simulation.type', str, 'transistor', None, '%s'),
    Parameter('current.step', int, 0, None, '%d'),
    Parameter('iterations.real', int, 1, None, '%d'),
    Parameter('random.seed', int, -1, None, '%d'),
    Parameter('grid.z', int, 1, None, '%d'),
    Parameter('grid.y', int, 1, None, '%d'),
    Parameter('grid.x', int, 1, None, '%d'),
    Parameter('hopping.range', int, 1, None, '%d'),
    Parameter('output.is.on', bool, True, None, '%s'),
    Parameter('iterations.print', int, 1, None, '%d'),
    Parameter('output.precision', int, 15, None, '%d'),
    Parameter('output.width', int, 23, None, '%d'),
    Parameter('output.stub', str, 'out', None, '%s'),
    Parameter('output.ids.on.delete', bool, False, None, '%s'),
    Parameter('output.ids.on.encounter', bool, False, None, '%s'),
    Parameter('output.coulomb', int, 0, None, '%d'),
    Parameter('output.step.chk', int, 1, None, '%d'),
    Parameter('output.chk.trap.potential', bool, False, None, '%s'),
    Parameter('output.potential', bool, False, None, '%s'),
    Parameter('output.xyz', int, 0, None, '%d'),
    Parameter('output.xyz.e', bool, True, None, '%s'),
    Parameter('output.xyz.h', bool, True, None, '%s'),
    Parameter('output.xyz.d', bool, True, None, '%s'),
    Parameter('output.xyz.t', bool, True, None, '%s'),
    Parameter('output.xyz.mode', int, 0, None, '%d'),
    Parameter('image.traps', bool, False, None, '%s'),
    Parameter('image.defects', bool, False, None, '%s'),
    Parameter('image.carriers', int, 0, None, '%d'),
    Parameter('electron.percentage', float, 0.0, None, '%.15e'),
    Parameter('hole.percentage', float, 0.0, None, '%.15e'),
    Parameter('seed.charges', float, 0.0, None, '%.15e'),
    Parameter('defect.percentage', float, 0.0, None, '%.15e'),
    Parameter('trap.percentage', float, 0.0, None, '%.15e'),
    Parameter('trap.potential', float, 0.0, None, '%.15e'),
    Parameter('gaussian.stdev', float, 0.0, None, '%.15e'),
    Parameter('seed.percentage', float, 1.0, None, '%.15e'),
    Parameter('voltage.right', float, 0.0, None, '%.15e'),
    Parameter('voltage.left', float, 0.0, None, '%.15e'),
    Parameter('slope.z', float, 0.0, None, '%.15e'),
    Parameter('coulomb.carriers', bool, False, None, '%s'),
    Parameter('coulomb.gaussian.sigma', float, 0.0, None, '%.15e'),
    Parameter('defects.charge', int, 0, None, '%d'),
    Parameter('exciton.binding', float, 0.0, None, '%.15e'),
    Parameter('temperature.kelvin', float, 300.0, None, '%.15e'),
    Parameter('source.rate', float, 0.9, None, '%.15e'),
    Parameter('e.source.l.rate', float, -1.0, None, '%.15e'),
    Parameter('e.source.r.rate', float, -1.0, None, '%.15e'),
    Parameter('h.source.l.rate', float, -1.0, None, '%.15e'),
    Parameter('h.source.r.rate', float, -1.0, None, '%.15e'),
    Parameter('generation.rate', float, 1.0e-3, None, '%.15e'),
    Parameter('source.metropolis', bool, False, None, '%s'),
    Parameter('source.coulomb', bool, False, None, '%s'),
    Parameter('source.scale.area', float, 65536.0, None, '%.15e'),
    Parameter('balance.charges', bool, False, None, '%s'),
    Parameter('drain.rate', float, 0.9, None, '%.15e'),
    Parameter('e.drain.l.rate', float, -1.0, None, '%.15e'),
    Parameter('e.drain.r.rate', float, -1.0, None, '%.15e'),
    Parameter('h.drain.l.rate', float, -1.0, None, '%.15e'),
    Parameter('h.drain.r.rate', float, -1.0, None, '%.15e'),
    Parameter('recombination.rate', float, 0.0, None, '%.15e'),
    Parameter('recombination.range', int, 0, None, '%d'),
    Parameter('use.opencl', bool, False, None, '%s'),
    Parameter('work.x', int, 4, None, '%d'),
    Parameter('work.y', int, 4, None, '%d'),
    Parameter('work.z', int, 4, None, '%d'),
    Parameter('work.size', int, 256, None, '%d'),
    Parameter('opencl.threshold', int, 256, None, '%d'),
    Parameter('opencl.device.id', int, 0, None, '%d'),
    Parameter('max.threads', int, -1, None, '%d')
]
parameters = collections.OrderedDict(((p.key, p) for p in parameters))

class Parameters(collections.OrderedDict):
    """
    A class to store Langmuir simulation parameters.
    """

    def __init__(self, handle=None):
        """
        Create Parameters instance.

        :param handle: filename or file object
        :type handle: str

        >>> parm = lm.parameters.Parameters('out.parm')
        """
        collections.OrderedDict.__init__(self)
        self.clear()
        if not handle is None:
            self.load(handle)

    def set_defaults(self):
        """
        Set parameters to default values.

        >>> parm = lm.parameters.Parameters()
        >>> parm.set_defaults()
        """
        for key, parameter in parameters.iteritems():
            self[key] = parameter.default

    def load(self, handle):
        """
        Load parameters from a file.

        :param handle: filename or file object
        :type handle: str

        >>> parm = lm.parameters.Parameters()
        >>> parm.load('out.parm')
        """
        handle = lm.common.zhandle(handle, 'rb')
        self.clear()
        line = handle.readline()
        while len(line) > 0:
            line = lm.regex.strip_comments(line).strip()
            line = lm.regex.fix_boolean(line)
            if line and not '[Parameters]' in line:
                tokens = [s.strip() for s in line.split('=')]
                key, value = tokens
                try:
                    value = eval(value)
                except NameError:
                    pass
                self[key] = value
            line = handle.readline()

    def save(self, handle):
        """
        Save parameters to a file.

        :param handle: filename or file object
        :type handle: str

        >>> parm = lm.parameters.Parameters()
        >>> parm.save('sim.inp')
        """
        handle = lm.common.zhandle(handle, 'wb')

        print >> handle, '[Parameters]'
        for key, value in self.iteritems():
            if not value is None:
                print >> handle, '%-25s = ' % key + str(self[key])

    def to_ndarray(self):
        """
        Convert parameters to a numpy array.
        """
        try:
            dtype = []
            for key, parameter in parameters.iteritems():
                if parameter.pytype == float:
                    dtype.append((parameter.key, np.float64))
                elif parameter.pytype == int:
                    dtype.append((parameter.key, np.int64))
                elif parameter.pytype == bool:
                    dtype.append((parameter.key, np.bool))
                elif parameter.pytype == str:
                    dtype.append((parameter.key, '|S1024'))
                else:
                    dtype.append((parameter.key, parameter.pytype))
            array = np.zeros(shape=(1,), dtype=dtype)
            for key, value in self.iteritems():
                if not value is None:
                    array[key] = value
            return array
        except NameError:
            raise RuntimeError('can not use to_ndarray without numpy')

    def to_series(self):
        """
        Convert parameters to a pandas series.
        """
        try:
            return pd.Series(self)
        except NameError:
            raise RuntimeError('can not use to_series without pandas')

    def to_dict(self):
        """
        Convert to simple python dict.
        """
        return dict(self)

    def __setitem__(self, key, value):
        try:
            if not key in parameters.keys():
                new_key = re.sub('_', '.', key)
            else:
                new_key = key
            assert new_key in parameters.keys()
        except AssertionError:
            raise KeyError('invalid parameter: %s' % key)
        super(Parameters, self).__setitem__(new_key, value)

    def __str__(self):
        s = StringIO.StringIO()
        for key, value in self.iteritems():
            print >> s, '%-25s = %s' % (key, value)
        return s.getvalue()

def load(handle):
    """
    Create Parameters object from file.

    :param handle: filename or file object
    :type handle: str
    """
    return Parameters(handle)

def compare(parm1, parm2):
    """
    Compare two Parameter objects.
    """
    return lm.common.compare_dicts(parm1, parm2)