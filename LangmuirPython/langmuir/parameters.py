# -*- coding: utf-8 -*-
import numpy as _np
import langmuir

_valid_keys = [col.name for col in langmuir.database.parameters]

class Parameters(dict):
    """
    A class to hold parameters in a dictionary.  Inherits :class:`'dict'.
    Will load parameters from a file if handle is not :py:obj:`None`.

    :param handle: filename or file object; Can be :py:obj:`None`.
    :type handle: str

    >>> parm = langmuir.parameters.Parameters('out.parm')
    """

    def __init__(self, handle=None):
        if not handle is None:
            self.load(handle)

    def load(self, handle):
        """
        Load parameters from a file.

        :param handle: filename or file object
        :type handle: str

        >>> parm = langmuir.parameters.Parameters()
        >>> parm.load('out.parm')
        """
        if type(handle) is str:
            handle = langmuir.common.zhandle(handle, 'rb')
        self.clear()
        line = handle.readline()
        while len(line) > 0:
            line = langmuir.regex.strip_comments(line).strip()
            line = langmuir.regex.fix_boolean(line)
            if line and not '[Parameters]' in line:
                tokens = [s.strip() for s in line.split('=')]
                key, value = tokens
                if not self._is_valid_key(key):
                    raise RuntimeError('encountered invalid key: %s' % key)
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

        >>> parm.save('sim.inp')
        """
        self._check_keys()
        if type(handle) is str:
            handle = langmuir.common.zhandle(handle, 'wb')

        print >> handle, '[Parameters]'
        for key in _valid_keys:
            if key in self.keys():
                print >> handle, '%-25s = ' % key + str(self[key])

    def __str__(self):
        self._check_keys()
        s = ''
        for key in _valid_keys:
            if key in self.keys():
                s += ('%-25s = ' % key + str(self[key]) + '\n')
        return s.rstrip()

    def reset_output_parameters(self):
        """
        Reset output parameters to typical values; Minimizes output
        without turning it off completely.

        >>> parm.reset_output_parameters()
        >>> print parm
        output.precision          = 15
        output.width              = 23
        output.stub               = out
        output.ids.on.delete      = False
        output.ids.on.encounter   = False
        output.coulomb            = 0
        output.step.chk           = 0
        output.potential          = False
        output.xyz                = 0
        output.xyz.e              = True
        output.xyz.h              = True
        output.xyz.d              = False
        output.xyz.t              = False
        output.xyz.mode           = 0
        image.traps               = False
        image.defects             = False
        image.carriers            = 0
        """
        self["output.precision"       ] = 15
        self["output.width"           ] = 23
        self["output.stub"            ] = "out"
        self["output.ids.on.delete"   ] = False
        self["output.ids.on.encounter"] = False
        self["output.coulomb"         ] = 0
        self["output.step.chk"        ] = 0
        self["output.potential"       ] = False
        self["output.xyz"             ] = 0
        self["output.xyz.e"           ] = True
        self["output.xyz.h"           ] = True
        self["output.xyz.t"           ] = False
        self["output.xyz.d"           ] = False
        self["output.xyz.mode"        ] = 0
        self["image.traps"            ] = False
        self["image.defects"          ] = False
        self["image.carriers"         ] = 0

    def set_defaults(self):
        """
        Set parameters to defaults found in database.

        >>> parm.set_defaults()
        >>> print parm
        simulation.type           = solarcell
        current.step              = 0
        iterations.real           = 10000
        random.seed               = 0
        grid.z                    = 1
        grid.y                    = 256
        grid.x                    = 256
        hopping.range             = 2
        output.is.on              = True
        iterations.print          = 1000
        output.precision          = 15
        output.width              = 23
        output.stub               = out
        output.ids.on.delete      = False
        output.ids.on.encounter   = False
        output.coulomb            = 0
        output.step.chk           = 10
        output.chk.trap.potential = False
        output.potential          = False
        output.xyz                = 0
        output.xyz.e              = True
        output.xyz.h              = True
        output.xyz.d              = False
        output.xyz.t              = False
        output.xyz.mode           = 0
        image.traps               = False
        image.defects             = False
        image.carriers            = 0
        electron.percentage       = 0.01
        hole.percentage           = 0.01
        seed.charges              = 0.0
        defect.percentage         = 0.0
        trap.percentage           = 0.0
        trap.potential            = 0.0
        gaussian.stdev            = 0.0
        seed.percentage           = 0.0
        voltage.right             = 0.0
        voltage.left              = 0.0
        exciton.binding           = 0.0
        slope.z                   = 0.0
        coulomb.carriers          = True
        coulomb.gaussian.sigma    = 1.0
        defects.charge            = 0
        temperature.kelvin        = 300.0
        source.rate               = 0.001
        e.source.l.rate           = -1.0
        e.source.r.rate           = -1.0
        h.source.l.rate           = -1.0
        h.source.r.rate           = -1.0
        generation.rate           = -1.0
        balance.charges           = False
        source.metropolis         = False
        source.coulomb            = False
        source.scale.area         = 65536.0
        drain.rate                = 0.9
        e.drain.l.rate            = -1.0
        e.drain.r.rate            = -1.0
        h.drain.l.rate            = -1.0
        h.drain.r.rate            = -1.0
        recombination.rate        = 0.0001
        recombination.range       = 0
        use.opencl                = True
        work.x                    = 4
        work.y                    = 4
        work.z                    = 4
        work.size                 = 256
        opencl.threshold          = 256
        opencl.device.id          = 0
        max.threads               = -1
        """
        for col in langmuir.database.parameters:
            self[col.name] = col.dflt
    
    def set_empty(self):
        for col in langmuir.database.parameters:
            if not col.name in self.keys():
                self[col.name] = col.dflt

    def _is_valid_key(self, key):
        return key in _valid_keys

    def _check_keys(self):
        for key in self.keys():
            if not self._is_valid_key(key):
                raise RuntimeError('encountered invalid key: %s' % key)

    def to_ndarray(self, result=None, rows=1, i=0):
        """
        Copy parameters into a :func:`np.ndarray`.

        :param result: in place array to be modified; if :py:obj:`None`, a new array is created.
        :param rows: number of rows to allocate in new array
        :param i: row-id to write paramters to

        :type result: :func:`numpy.array`
        :type rows: int
        :type i: int

        :returns: :func:`numpy.array`

        >>> a = parm.to_ndarray()
        """
        if result is None:
            result = create_ndarray(rows=rows)
        for key in self.keys():
            result[key][i] = self[key]
        return result

def load(handle):
    """
    Load parameters from file.

    :param handle: filename or file object
    :type handle: str

    :returns: :class:`Parameters`

    >>> parm = langmuir.paramters.load('out.parm')
    """
    return Parameters(handle)

def create_ndarray(rows=0):
    """
    Create empty :func:`numpy.array` with correct column headers.

    :param rows: number of rows to allocate in new array
    :type rows: int

    :returns: :func:`numpy.array`

    >>> a = create_ndarray(10)
    """
    return _np.zeros(shape=(rows,), dtype=langmuir.database.parameters.dtype)
