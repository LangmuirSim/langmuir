# -*- coding: utf-8 -*-
import collections as _collections
import langmuir

class CheckPoint:
    """
    A class to open langmuir checkpoint files.

    ========================= =======================================
    **Attribute**             **Description**
    ========================= =======================================
    :py:attr:`electrons`      :py:obj:`list` of :py:obj:`int`
    :py:attr:`holes`          :py:obj:`list` of :py:obj:`int`
    :py:attr:`traps`          :py:obj:`list` of :py:obj:`int`
    :py:attr:`defects`        :py:obj:`list` of :py:obj:`int`
    :py:attr:`trapPotentials` :py:obj:`list` of :py:obj:`float`
    :py:attr:`fluxState`      :py:obj:`list` of :py:obj:`int`
    :py:attr:`randomState`    :py:obj:`list` of :py:obj:`int`
    :py:attr:`parameters`     :class:`Parameters`
    ========================= =======================================

    :param handle: filename or file object; Can be :py:obj:`None`.
    :type handle: str
    """

    def __init__(self, handle=None):
        self.electrons      = []
        self.holes          = []
        self.traps          = []
        self.defects        = []
        self.trapPotentials = []
        self.fluxState      = []
        self.randomState    = []
        self.parameters     = langmuir.parameters.Parameters()
        self.path           = None
        if not handle is None:
            self.load(handle)

    def load(self, handle):
        """
        Load checkpoint from a file.

        :param handle: filename or file object
        :type handle: str

        >>> chk = langmuir.checkpoint.CheckPoint()
        >>> chk.load('out.chk')
        """
        self.clear()
        if type(handle) is str:
            self.path = handle
            handle = langmuir.common.zhandle(handle, 'rb')
        else:
            self.path = handle.name
        line = handle.readline()
        while line:
            line = line.strip()
            if line == '[Electrons]':
                self.electrons = self._load_values(handle, int)
            elif line == '[Holes]':
                self.holes = self._load_values(handle, int)
            elif line == '[Defects]':
                self.defects = self._load_values(handle, int)
            elif line == '[Traps]':
                self.traps = self._load_values(handle, int)
            elif line == '[TrapPotentials]':
                self.trapPotentials = self._load_values(handle, float)
            elif line == '[FluxState]':
                self.fluxState = self._load_values(handle, int)
            elif line == '[RandomState]':
                self.randomState = [int(i.strip()) for i in
                    handle.readline().strip().split()]
            elif line == '[Parameters]':
                self.parameters.load(handle)
            else:
                raise RuntimeError('invalid section:\n\t%s' % line)
            line = handle.readline()
        handle.close()

    def save(self, handle):
        """
        Save checkpoint to a file.

        :param handle: filename or file object
        :type handle: str

        >>> chk.save('sim.inp')
        """
        if type(handle) is str:
            handle = langmuir.common.zhandle(handle, 'wb')
        if self.electrons:
            self._save_label(handle, 'Electrons')
            self._save_values(handle, self.electrons)
        if self.holes:
            self._save_label(handle, 'Holes')
            self._save_values(handle, self.holes)
        if self.defects:
            self._save_label(handle, 'Defects')
            self._save_values(handle, self.defects)
        if self.traps:
            self._save_label(handle, 'Traps')
            self._save_values(handle, self.traps)
        if self.trapPotentials:
            self._save_label(handle, 'TrapPotentials')
            self._save_values(handle, self.trapPotentials)
        if self.fluxState:
            self._save_label(handle, 'FluxState')
            self._save_values(handle, self.fluxState)
        if self.randomState:
            self._save_label(handle, 'RandomState')
            print >> handle, ' '.join(('%d' % i for i in self.randomState))
        if self.parameters:
            self.parameters.save(handle)
        handle.close()

    def clear(self):
        """
        Forget all stored information.

        >>> chk.clear()
        >>> print chk
        [Electrons]     : 0
        [Holes]         : 0
        [Traps]         : 0
        [Defects]       : 0
        [TrapPotenials] : 0
        [FluxState]     : 0
        [RandomState]   : 0
        [Parameters]    : 0
        """
        self.electrons      = []
        self.holes          = []
        self.traps          = []
        self.defects        = []
        self.trapPotentials = []
        self.fluxState      = []
        self.randomState    = []
        self.parameters.clear()

    def reset(self, keep_elecs=False, keep_holes=False):
        """
        Reset current step to zero, clear random state, clear flux state,
        and delete charge carriers.

        :param keep_elecs: do not delete electrons
        :param keep_holes: do not delete holes

        :type keep_elecs: bool
        :type keep_holes: bool

        >>> chk.reset()
        >>> print chk
        [Electrons]     : 0
        [Holes]         : 0
        [Traps]         : 1000
        [Defects]       : 1000
        [TrapPotenials] : 1000
        [FluxState]     : 0
        [RandomState]   : 0
        [Parameters]    : 10
        ...
        """
        if not keep_elecs:
            self.electrons = []
        if not keep_holes:
            self.holes = []
        self.parameters['current.step'] = 0
        self.parameters['random.seed'] = 0
        self.randomState = []
        self.fluxState = []

    def _load_values(self, handle, type_=int):
        """
        read lines from handle and convert them to type_
        """
        count = int(handle.readline().strip())
        values = [type_(handle.readline().strip()) for i in range(count)]
        return values

    def _save_values(self, handle, values):
        """
        write values to handle
        """
        print >> handle, len(values)
        for v in values:
            print >> handle, v

    def _save_label(self, handle, label):
        """
        write section header to handle
        """
        print >> handle, '[%s]' % label

    def __str__(self):
        s  = ''
        s += '[Electrons]     : %d\n' % len(self.electrons)
        s += '[Holes]         : %d\n' % len(self.holes)
        s += '[Traps]         : %d\n' % len(self.traps)
        s += '[Defects]       : %d\n' % len(self.defects)
        s += '[TrapPotenials] : %d\n' % len(self.trapPotentials)
        s += '[FluxState]     : %d\n' % len(self.fluxState)
        s += '[RandomState]   : %d\n' % len(self.randomState)
        s += '[Parameters]    : %d\n' % len(self.parameters)
        s += str(self.parameters)
        return s

    def fix_traps(self):
        """
        Check trap parameter validity and remove extra or inconsistant information.

        >>> chk.fix_traps()
        """
        if self.traps:

            size = len(self.traps)

            if self.trapPotentials:
                if not len(self.trapPotentials) == size:
                    raise RuntimeError('number of traps and potentials ' +
                                       'do not agree')

                value = self.trapPotentials[0]

                count = self.trapPotentials.count(value)
                if not count == len(self.trapPotentials):
                    value = None
            else:
                value = self.parameters['trap.potential']

            if not self.parameters.has_key('grid.x'):
                self.parameters['grid.x'] = 1

            if not self.parameters.has_key('grid.y'):
                self.parameters['grid.y'] = 1

            if not self.parameters.has_key('grid.z'):
                self.parameters['grid.z'] = 1

            volume = self.parameters['grid.x'] * \
                     self.parameters['grid.y'] * \
                     self.parameters['grid.z']

            if not size <= volume:
                raise RuntimeError('number of traps exceeds volume: %d > %d'
                    % (size, volume))

            percentage = (size + 0.25) / volume

            self.parameters['trap.percentage'] = percentage
            self.parameters['seed.percentage'] = 0

            if value is None:
                self.parameters['trap.potential'] = 0.0
            else:
                self.trapPotentials = []
                self.parameters['trap.potential'] = value

    def __getitem__(self, key):
        """
        Access checkpoint attribute or parameters by key
        """
        if hasattr(self, key):
            return getattr(self, key)
        elif key in langmuir.parameters._valid_keys:
            return self.parameters[key]
        raise AttributeError('invalid attribute or key : %s' % key)

    def __setitem__(self, key, value):
        """
        Access checkpoint attribute or parameters by key
        """
        if hasattr(self, key):
            setattr(self, key, value)
            return
        else:
            if key in langmuir.parameters._valid_keys:
                self.parameters[key] = value
                return
        raise AttributeError('invalid attribute or key: %s' % key)

def load(handle):
    """
    Load checkpoint file.

    :param handle: filename or file object
    :type handle: str

    >>> chk = langmuir.checkpoint.load('out.chk')
    """
    return CheckPoint(handle)

def load_last(work, **kwargs):
    """
    Load the last checkpoint file found in the working directory.

    :param work: directory to look in
    :type work: str

    chk = langmuir.checkpoint.load_last('/home/adam/Desktop/simulations')
    """
    if not kwargs.has_key('at_least_one'):
        kwargs['at_least_one'] = True

    if not kwargs.has_key('recursive'):
        kwargs['recursive'] = True

    last = langmuir.find.chks(work, **kwargs)[-1]
    return load(last)

def compare(chk_i, chk_j):
    """
    Rigorously compare to checkpoint files.

    :param chk_i: checkpoint object 1
    :param chk_j: checkpoint object 1

    :type chk_i: langmuir.checkpoint.CheckPoint
    :type chk_j: langmuir.checkpoint.CheckPoint

    :returns: dict, bool
    """
    results = _collections.OrderedDict()

    try:
        set_i = set(chk_i.electrons)
        set_j = set(chk_j.electrons)
        inter = set_i.intersection(set_j)
        union = set_i.union(set_j)
        results['elecs'] = _collections.OrderedDict()
        results['elecs']['size1'] = len(set_i)
        results['elecs']['size2'] = len(set_j)
        results['elecs']['union'] = len(union)
        results['elecs']['inter'] = len(inter)
        assert set_i == set_j
        results['elecs']['valid'] = True
    except AssertionError:
        results['elecs']['valid'] = False

    try:
        set_i = set(chk_i.holes)
        set_j = set(chk_j.holes)
        inter = set_i.intersection(set_j)
        union = set_i.union(set_j)
        results['holes'] = _collections.OrderedDict()
        results['holes']['size1'] = len(set_i)
        results['holes']['size2'] = len(set_j)
        results['holes']['union'] = len(union)
        results['holes']['inter'] = len(inter)
        assert set_i == set_j
        results['holes']['valid'] = True
    except AssertionError:
        results['holes']['valid'] = False

    try:
        set_i = set(chk_i.defects)
        set_j = set(chk_j.defects)
        inter = set_i.intersection(set_j)
        union = set_i.union(set_j)
        results['defects'] = _collections.OrderedDict()
        results['defects']['size1'] = len(set_i)
        results['defects']['size2'] = len(set_j)
        results['defects']['union'] = len(union)
        results['defects']['inter'] = len(inter)
        assert set_i == set_j
        results['defects']['valid'] = True
    except AssertionError:
        results['defects']['valid'] = False

    try:
        set_i = set(chk_i.traps)
        set_j = set(chk_j.traps)
        inter = set_i.intersection(set_j)
        union = set_i.union(set_j)
        results['traps'] = _collections.OrderedDict()
        results['traps']['size1'] = len(set_i)
        results['traps']['size2'] = len(set_j)
        results['traps']['union'] = len(union)
        results['traps']['inter'] = len(inter)
        assert set_i == set_j
        results['traps']['valid'] = True
    except AssertionError:
        results['traps']['valid'] = False

    try:
        set_i = set(chk_i.trapPotentials)
        set_j = set(chk_j.trapPotentials)
        inter = set_i.intersection(set_j)
        union = set_i.union(set_j)
        results['potentials'] = _collections.OrderedDict()
        results['potentials']['size1'] = len(set_i)
        results['potentials']['size2'] = len(set_j)
        results['potentials']['union'] = len(union)
        results['potentials']['inter'] = len(inter)
        assert set_i == set_j
        results['potentials']['valid'] = True
    except AssertionError:
        results['potentials']['valid'] = False

    try:
        set_i = set(chk_i.fluxState)
        set_j = set(chk_j.fluxState)
        inter = set_i.intersection(set_j)
        union = set_i.union(set_j)
        results['flux'] = _collections.OrderedDict()
        results['flux']['size1'] = len(set_i)
        results['flux']['size2'] = len(set_j)
        results['flux']['union'] = len(union)
        results['flux']['inter'] = len(inter)
        assert set_i == set_j
        results['flux']['valid'] = True
    except AssertionError:
        results['flux']['valid'] = False

    try:
        set_i = set(chk_i.randomState)
        set_j = set(chk_j.randomState)
        inter = set_i.intersection(set_j)
        union = set_i.union(set_j)
        results['random'] = _collections.OrderedDict()
        results['random']['size1'] = len(set_i)
        results['random']['size2'] = len(set_j)
        results['random']['union'] = len(union)
        results['random']['inter'] = len(inter)
        assert set_i == set_j
        results['random']['valid'] = True
    except AssertionError:
        results['random']['valid'] = False

    try:
        diff = langmuir.common.DictDiffer
        d1, d2 = chk_i.parameters, chk_j.parameters
        set_i = set(d1.keys())
        set_j = set(d2.keys())
        inter = set_i.intersection(set_j)
        union = set_i.union(set_j)
        results['parameters'] = _collections.OrderedDict()
        results['parameters']['size1'] = len(set_i)
        results['parameters']['size2'] = len(set_j)
        results['parameters']['union'] = len(union)
        results['parameters']['inter'] = len(inter)

        changed = diff.changedValues(d1, d2)

        for key, (v1, v2) in changed.iteritems():
            results['parameters'][key] = (v1, v2)

        assert set_i == set_j
        assert len(changed) == 0

        results['parameters']['valid'] = True
    except AssertionError:
        results['parameters']['valid'] = False

    same = True
    for k1, v1 in results.iteritems():
        if not v1['valid']:
            same = False

    return same, results
