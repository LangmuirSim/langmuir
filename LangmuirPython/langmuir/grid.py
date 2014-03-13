# -*- coding: utf-8 -*-
"""
.. note::
    Functions for performing calculations on grids.

.. moduleauthor:: Adam Gagorik <adam.gagorik@gmail.com>
"""
import langmuir as lm
import numpy as np
import itertools
import StringIO
import warnings

class Grid(object):
    """
    A class to represent the Langmuir simulation lattice.  Alternate
    contructors exist.

    :param xsize: x points
    :param ysize: y points
    :param zsize: z points

    :type xsize: int
    :type ysize: int
    :type zsize: int
    """

    def __init__(self, xsize, ysize, zsize):
        self._x0 = 0.0
        self._y0 = 0.0
        self._z0 = 0.0

        self._x1 = float(xsize)
        self._y1 = float(ysize)
        self._z1 = float(zsize)

        self._lx = abs(self._x1 - self._x0)
        self._ly = abs(self._y1 - self._y0)
        self._lz = abs(self._z1 - self._z0)

        self._dx = 1.0
        self._dy = 1.0
        self._dz = 1.0

        self._hx = 0.5 * self._dx
        self._hy = 0.5 * self._dy
        self._hz = 0.5 * self._dz

        self._nx = int(xsize)
        self._ny = int(ysize)
        self._nz = int(zsize)

        self._n_xy = self._nx * self._ny
        self._n_xz = self._nx * self._nz
        self._n_yz = self._ny * self._nz

        self._mx = None
        self._my = None
        self._mz = None

        self._ox = None
        self._oy = None
        self._oz = None

    @classmethod
    def from_checkpoint(cls, chk):
        """
        Create grid instance from checkpoint file.

        :param chk: checkpoint filename or object

        >>> grid = lm.grid.Grid.from_checkpoint('out.chk')
        """
        chk = lm.checkpoint.load(chk)
        nx, ny, nz = 1, 1, 1
        if chk.has_key('grid.x'): nx = chk['grid.x']
        if chk.has_key('grid.y'): ny = chk['grid.y']
        if chk.has_key('grid.z'): nz = chk['grid.z']
        return cls(nx, ny, nz)

    def create_zeros(self):
        """
        Compute a grid of zeros.
        """
        return np.zeros(self.shape)

    def parameters(self):
        """
        Return a :class:`langmuir.parameters.Parameters`.
        """
        parm = lm.parameters.Parameters()
        parm['grid.x'] = self.nx
        parm['grid.y'] = self.ny
        parm['grid.z'] = self.nz
        return parm

    def refine(self, factor=0):
        """
        Refines the mesh.  If factor=0 or dx/factor > 1.0, the mesh is reset
        to the default spacing of 1.0.

        :param factor: zoom factor; > 0 refines the mesh, < 0 zooms out.
        :type factor: float
        """
        if factor == 1:
            return

        if factor <= 0:
            dx = 1.0
            dy = 1.0
            dz = 1.0
        else:
            dx = self._dx / float(factor)
            dy = self._dy / float(factor)
            dz = self._dz / float(factor)

        if any([dx >= 1.0, dy >= 1.0, dz >= 1.0]):
            dx = 1.0
            dy = 1.0
            dz = 1.0

        assert dx == dy
        assert dy == dz

        if self._dx == dx:
            return

        self._dx = dx
        self._dy = dy
        self._dz = dz

        self._hx = 0.5 * self._dx
        self._hy = 0.5 * self._dy
        self._hz = 0.5 * self._dz

        self._nx = int(self._lx / self._dx)
        self._ny = int(self._ly / self._dy)
        self._nz = int(self._lz / self._dz)

        self._n_xy = self._nx * self._ny
        self._n_xz = self._nx * self._nz
        self._n_yz = self._ny * self._nz

        self._mx = None
        self._my = None
        self._mz = None

        self._ox = None
        self._oy = None
        self._oz = None

    def _create_ogrid(self):
        self._ox = np.array(
            [self._x0 + i * self._dx for i in range(self._nx)]) + self._hx
        self._oy = np.array(
            [self._y0 + i * self._dy for i in range(self._ny)]) + self._hy
        self._oz = np.array(
            [self._z0 + i * self._dz for i in range(self._nz)]) + self._hz

    def _create_mgrid(self):
        if self._ox is None:
            self._create_ogrid()
        self._mx, self._my, self._mz = \
            np.meshgrid(self._ox, self._oy, self._oz, indexing='ij')

    @property
    def x0(self):
        return self._x0
    @property
    def y0(self):
        return self._y0
    @property
    def z0(self):
        return self._z0

    @property
    def x1(self):
        return self._x1
    @property
    def y1(self):
        return self._y1
    @property
    def z1(self):
        return self._z1

    @property
    def nx(self):
        return self._nx
    @property
    def ny(self):
        return self._ny
    @property
    def nz(self):
        return self._nz

    @property
    def n_xy(self):
        return self._n_xy
    @property
    def n_yz(self):
        return self._n_yz
    @property
    def n_xz(self):
        return self._n_xz

    @property
    def dx(self):
        return self._dx
    @property
    def dy(self):
        return self._dy
    @property
    def dz(self):
        return self._dz

    @property
    def hx(self):
        return self._hx
    @property
    def hy(self):
        return self._hy
    @property
    def hz(self):
        return self._hz

    @property
    def lx(self):
        return self._lx
    @property
    def ly(self):
        return self._ly
    @property
    def lz(self):
        return self._lz

    @property
    def mx(self):
        if self._mx is None:
            self._create_mgrid()
        return self._mx
    @property
    def my(self):
        if self._my is None:
            self._create_mgrid()
        return self._my
    @property
    def mz(self):
        if self._mz is None:
            self._create_mgrid()
        return self._mz
    @property
    def mgrid(self):
        x = self.mx
        y = self.my
        z = self.mz
        return x, y, z

    @property
    def ox(self):
        if self._ox is None:
            self._create_ogrid()
        return self._ox
    @property
    def oy(self):
        if self._oy is None:
            self._create_ogrid()
        return self._oy
    @property
    def oz(self):
        if self._oz is None:
            self._create_ogrid()
        return self._oz
    @property
    def ogrid(self):
        x = self.ox
        y = self.oy
        z = self.oz
        return x, y, z

    @property
    def size(self):
        return self._nx * self._ny * self._nz

    @property
    def shape(self):
        return self._nx, self._ny, self._nz

    @property
    def extent(self):
        return 0, self._nx - 1, 0, self._ny - 1, 0, self._nz - 1

    @property
    def bounds(self):
        return 0, self._nx, 0, self._ny, 0, self._nz

    @property
    def origin(self):
        return self._x0, self._y0, self._z0

    @property
    def spacing(self):
        return self._dx, self._dy, self._dz

    def __str__(self):
        s = StringIO.StringIO()
        w = 9
        print >> s, '[Grid]'
        print >> s, '{0:>12} = {1:>{w}.3e} {2:>{w}.3e} {3:>{w}.3e}'.format(
            'x0, y0, z0', self.x0, self.y0, self.z0, w=w)
        print >> s, '{0:>12} = {1:>{w}.3e} {2:>{w}.3e} {3:>{w}.3e}'.format(
            'x1, y1, z1', self.x1, self.y1, self.z1, w=w)
        print >> s, '{0:>12} = {1:>{w}.3e} {2:>{w}.3e} {3:>{w}.3e}'.format(
            'lx, ly, lz', self.lx, self.ly, self.lz, w=w)
        print >> s, '{0:>12} = {1:>{w}.3e} {2:>{w}.3e} {3:>{w}.3e}'.format(
            'dx, dy, dz', self.dx, self.dy, self.dz, w=w)
        print >> s, '{0:>12} = {1:>{w}} {2:>{w}} {3:>{w}}'.format(
            'nx, ny, nz', self.nx, self.ny, self.nz, w=w)
        print >> s, '{0:>12} = {1:>{w}}'.format('N', self.size, w=w)
        return s.getvalue()

class IndexMapper(object):
    """
    A class that maps between site indicies the Langmuir way.

    >>> grid = lm.grid.Grid(10, 10, 10)
    >>> imap = lm.grid.IndexMapper(grid)
    """
    def __init__(self, grid):
        self.grid = grid

    def indexS(self, x_index, y_index, z_index):
        """
        Compute (Langmuir's) site index from x, y, and z index

        :param x_index: x-site
        :param y_index: y-site
        :param z_index: z-site
        :type x_index: int
        :type y_index: int
        :type z_index: int

        >>> grid = lm.grid.Grid(10, 10, 10)
        >>> imap = lm.grid.IndexMapper(grid)
        >>> s = imap.indexS(0, 0, 0)
        """
        x_index = np.asarray(x_index)
        y_index = np.asarray(y_index)
        z_index = np.asarray(z_index)
        return self.grid.nx * (y_index + z_index * self.grid.ny) + x_index

    def indexX(self, s_index):
        """
        Compute (Langmuir's) x index from site index

        :param s_index: site index
        :type s_index: int

        >>> grid = lm.grid.Grid(10, 10, 10)
        >>> imap = lm.grid.IndexMapper(grid)
        >>> x = imap.indexX(10)
        """
        s_index = np.asarray(s_index)
        return s_index % self.grid.nx

    def indexY(self, s_index):
        """
        Compute (Langmuir's) y index from site index

        :param s_index: site index
        :type s_index: int

        >>> grid = lm.grid.Grid(10, 10, 10)
        >>> imap = lm.grid.IndexMapper(grid)
        >>> y = imap.indexX(10)
        """
        s_index = np.asarray(s_index)
        return s_index / self.grid.nx - \
            (s_index / self.grid.n_xy) * self.grid.ny

    def indexZ(self, s_index):
        """
        Compute (Langmuir's) z index from site index

        :param s_index: site index
        :type s_index: int

        >>> grid = lm.grid.Grid(10, 10, 10)
        >>> imap = lm.grid.IndexMapper(grid)
        >>> z = imap.indexX(10)
        """
        s_index = np.asarray(s_index)
        return s_index / self.grid.n_xy

    @staticmethod
    def map_mesh(grid, mesh, value=1.0):
        imap  = IndexMapper(grid)
        traps = []
        for i, ((xi, yi, zi), vi) in enumerate(np.ndenumerate(mesh)):
            if vi == value:
                s = imap.indexS(xi, yi, zi)
                traps.append(s)
        return traps

class XYZV:
    """
    Put site values on a mesh using site ids.

    :param grid: grid object
    :param s: site indcies
    :param v: site values

    :type grid: :class:`Grid`
    :type s: list
    :type v: list or scalar

    >>> chk  = lm.checkpoint.load('out.chk')
    >>> grid = lm.grid.Grid.from_checkpoint(chk)
    >>> xyzv = lm.grid.XYZV(grid, chk.electrons, -1)
    """
    def __init__(self, grid, s, v=None):

        self.grid = grid
        self.imap = IndexMapper(self.grid)

        self.s = np.asarray(s)

        if v is None:
            self.v = np.ones(self.s.size, dtype=float)

        elif np.isscalar(v):
            self.v = np.ones(self.s.size, dtype=float)
            self.v = self.v * v

        else:
            self.v = np.asarray(v)

        if not self.v.size == self.v.size:
            raise RuntimeError('site ids and values are not the same size')

        self.xi = self.imap.indexX(self.s)
        self.yi = self.imap.indexY(self.s)
        self.zi = self.imap.indexZ(self.s)

        self.mx = grid.mx
        self.my = grid.my
        self.mz = grid.mz
        self.mv = grid.create_zeros()

        self.x = self.mx[self.xi, self.yi, self.zi]
        self.y = self.my[self.xi, self.yi, self.zi]
        self.z = self.mz[self.xi, self.yi, self.zi]

        self.mv[self.xi, self.yi, self.zi] = self.v
        self.xyz = np.dstack([self.x, self.y, self.z]).flatten()

    def x_at_zi_equals(self, zlevel=0):
        return self.mx[self.xi, self.yi, zlevel]
    def y_at_zi_equals(self, zlevel=0):
        return self.my[self.xi, self.yi, zlevel]
    def z_at_zi_equals(self, zlevel=0):
        return self.mz[self.xi, self.yi, zlevel]
    def v_at_zi_equals(self, zlevel=0):
        return self.mv[self.xi, self.yi, zlevel]

    def mx_at_zi_equals(self, zlevel=0):
        return self.mx[:,:, zlevel]
    def my_at_zi_equals(self, zlevel=0):
        return self.my[:,:, zlevel]
    def mz_at_zi_equals(self, zlevel=0):
        return self.mz[:,:, zlevel]
    def mv_at_zi_equals(self, zlevel=0):
        return self.mv[:,:, zlevel]

class PrecalculatedMesh:
    """
    Perform fast computation of Coulomb interactions and distances with a
    precomputed mesh.

    :param grid: grid object
    :type grid: :class:`Grid`

    >>> grid = lm.grid.Grid(5, 5, 5)
    >>> mesh = lm.grid.Mesh(grid)
    """
    def __init__(self, grid):
        self.grid = grid
        self.mx = grid.mx
        self.my = grid.my
        self.mz = grid.mz
        self.r2 = (self.mx - self.mx[0,0,0])**2 + \
                  (self.my - self.my[0,0,0])**2 + \
                  (self.mz - self.mz[0,0,0])**2
        self.r1 = np.sqrt(self.r2)

        with warnings.catch_warnings():
            warnings.simplefilter(action='ignore', category=RuntimeWarning)
            self.ri = 1.0 / self.r1
            self.ri[0, 0, 0] = 0

        self.mxi, self.myi, self.mzi = np.indices(self.grid.shape)
        self.ids = [(x_id, y_id, z_id)
            for (x_id, y_id, z_id), v in np.ndenumerate(self.mx)]

    def coulomb(self, xi_ids, yi_ids, zi_ids, xj_ids=None, yj_ids=None,
                zj_ids=None, q=1):
        """
        Compute coulomb interaction at j's due to charges at i's.  If no
        j's are passed, then the answer will be computed at every mesh
        point (expensive!)'

        :param xi_ids: charge x-position(s)
        :param yi_ids: charge y-position(s)
        :param zi_ids: charge z-position(s)
        :param xj_ids: energy x-position(s)
        :param yj_ids: energy y-position(s)
        :param zj_ids: energy z-position(s)
        :param q: charge
        :type xi_ids: list, int
        :type yi_ids: list, int
        :type zi_ids: list, int
        :type xi_ids: list, int
        :type yi_ids: list, int
        :type zi_ids: list, int
        :type q: int, float

        >>> grid = lm.grid.Grid(10, 10, 10)
        >>> mesh = lm.grid.PrecalculatedMesh(grid)
        >>> coul = mesh.coulomb(0, 0, 0, 1, 1, 1, -1)
        """
        xi_ids = np.asarray(xi_ids)
        yi_ids = np.asarray(yi_ids)
        zi_ids = np.asarray(zi_ids)
        assert xi_ids.size == yi_ids.size == zi_ids.size
        zipper = itertools.izip(xi_ids.flat, yi_ids.flat, zi_ids.flat)

        if xj_ids is None and yj_ids is None and zj_ids is None:
            xj_ids = self.mxi
            yj_ids = self.myi
            zj_ids = self.mzi

        xj_ids = np.asarray(xj_ids)
        yj_ids = np.asarray(yj_ids)
        zj_ids = np.asarray(zj_ids)
        assert xj_ids.size == yj_ids.size == zj_ids.size

        potential = np.zeros_like(xj_ids)
        for (xi_id, yi_id, zi_id) in zipper:
            dx_ids = abs(xj_ids - xi_id)
            dy_ids = abs(yj_ids - yi_id)
            dz_ids = abs(zj_ids - zi_id)
            potential = potential + self.ri[dx_ids, dy_ids, dz_ids]

        return q * potential

    def distances(self, xi_ids, yi_ids, zi_ids, xj_ids=None, yj_ids=None, zj_ids=None):
        """
        Compute all distances between i's, or compute all distances between
        i's and j's

        :param xi_ids: initial x-position(s)
        :param yi_ids: initial y-position(s)
        :param zi_ids: initial z-position(s)
        :param xj_ids: final x-position(s)
        :param yj_ids: final y-position(s)
        :param zj_ids: final z-position(s)
        :type xi_ids: list, int
        :type yi_ids: list, int
        :type zi_ids: list, int
        :type xi_ids: list, int
        :type yi_ids: list, int
        :type zi_ids: list, int

        >>> grid = lm.grid.Grid(10, 10, 10)
        >>> mesh = lm.grid.PrecalculatedMesh(grid)
        >>> dist = mesh.distances(0, 0, 0, 1, 1, 1)
        """
        if not xj_ids:
            xj_ids = []

        if not yj_ids:
            yj_ids = []

        if not zj_ids:
            zj_ids = []

        xi_ids = np.asarray(xi_ids)
        yi_ids = np.asarray(yi_ids)
        zi_ids = np.asarray(zi_ids)
        assert xi_ids.size == yi_ids.size == zi_ids.size
        sites1 = itertools.izip(xi_ids.flat, yi_ids.flat, zi_ids.flat)

        xj_ids = np.asarray(xj_ids)
        yj_ids = np.asarray(yj_ids)
        zj_ids = np.asarray(zj_ids)
        assert xj_ids.size == yj_ids.size == zj_ids.size

        if xj_ids.size:
            sites2 = itertools.izip(xj_ids.flat, yj_ids.flat, zj_ids.flat)
            zipper = itertools.product(sites1, sites2)
            size   = xi_ids.size * xj_ids.size
        else:
            zipper = itertools.combinations(sites1, 2)
            size   = lm.common.comb(xi_ids.size, 2, exact=1)

        distances = np.zeros(size)
        for i, (p1, p2) in enumerate(zipper):
            dx_id = abs(p2[0] - p1[0])
            dy_id = abs(p2[1] - p1[1])
            dz_id = abs(p2[2] - p1[2])
            distances[i] = self.r1[dx_id, dy_id, dz_id]

        return distances
