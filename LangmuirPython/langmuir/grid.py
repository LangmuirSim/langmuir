# -*- coding: utf-8 -*-
from langmuir.common import comb
import itertools as _itertools
import StringIO as _StringIO
import warnings as _warnings
import numpy as _np
import langmuir

class IndexMapper:
    """
    Compute (Langmuir's) site indices
    """
    def __init__(self, shape=(1, 1, 1)):
        self.shape  = shape
        self.xsize  = shape[0]
        self.ysize  = shape[1]
        self.zsize  = shape[2]
        self.size   = self.xsize * self.ysize * self.zsize
        self.xyarea = self.xsize * self.ysize

    def indexS(self, x_index, y_index, z_index):
        """
        Compute (langmuir's) site index from x, y, and z index
        """
        x_index = _np.asarray(x_index)
        y_index = _np.asarray(y_index)
        z_index = _np.asarray(z_index)
        return self.xsize * (y_index + z_index * self.ysize) + x_index

    def indexX(self, s_index):
        """
        Compute (langmuir's) x index from site index
        """
        s_index = _np.asarray(s_index)
        return s_index % self.xsize

    def indexY(self, s_index):
        """
        Compute (langmuir's) y index from site index
        """
        s_index = _np.asarray(s_index)
        return s_index / self.xsize - (s_index / self.xyarea) * self.ysize

    def indexZ(self, s_index):
        """
        Compute (langmuir's) z index from site index
        """
        s_index = _np.asarray(s_index)
        return s_index / self.xyarea

class Mesh:
    """
    Precompute distances on a mesh
    """
    def __init__(self, grid):
        """
        shape   : the dimensions of the grid (xsize, ysize, zsize)
        spacing : the spacing between mesh points
        origin  : the position of the origin  = front bottom left of cube
        """
        self.x, self.y, self.z = grid.create_mgrid()
        self.grid = grid

        self.r2 = (self.x - self.grid.x0)**2 +\
                  (self.y - self.grid.y0)**2 +\
                  (self.z - self.grid.z0)**2
        self.r1 = _np.sqrt(self.r2)

        with _warnings.catch_warnings():
            _warnings.simplefilter(action='ignore', category=RuntimeWarning)
            self.ri = 1 / self.r1
            self.ri[0, 0, 0] = 0

        self.x_ids, self.y_ids, self.z_ids = _np.indices(self.grid.shape)
        self.ids = [(x_id, y_id, z_id)
            for (x_id, y_id, z_id), v in _np.ndenumerate(self.x)]

    def coulomb(self, xi_ids, yi_ids, zi_ids, xj_ids=None, yj_ids=None,
                zj_ids=None, q=1):
        """
        Compute coulomb interaction at j's due to charges at i's.  If no
        j's are passed, then the answer will be computed at every mesh
        point (expensive!)'
        """
        xi_ids = _np.asarray(xi_ids)
        yi_ids = _np.asarray(yi_ids)
        zi_ids = _np.asarray(zi_ids)
        assert xi_ids.size == yi_ids.size == zi_ids.size
        zipper = _itertools.izip(xi_ids.flat, yi_ids.flat, zi_ids.flat)

        if xj_ids is None and yj_ids is None and zj_ids is None:
            xj_ids = self.x_ids
            yj_ids = self.y_ids
            zj_ids = self.z_ids

        xj_ids = _np.asarray(xj_ids)
        yj_ids = _np.asarray(yj_ids)
        zj_ids = _np.asarray(zj_ids)
        assert xj_ids.size == yj_ids.size == zj_ids.size

        potential = _np.zeros_like(xj_ids)
        for (xi_id, yi_id, zi_id) in zipper:
            dx_ids = abs(xj_ids - xi_id)
            dy_ids = abs(yj_ids - yi_id)
            dz_ids = abs(zj_ids - zi_id)
            potential = potential + self.ri[dx_ids, dy_ids, dz_ids]

        return q * potential

    def distances(self, xi_ids, yi_ids, zi_ids, xj_ids=[], yj_ids=[],
                  zj_ids=[]):
        """
        Compute all distances between i's, or compute all distances between
        i's and j's
        """
        xi_ids = _np.asarray(xi_ids)
        yi_ids = _np.asarray(yi_ids)
        zi_ids = _np.asarray(zi_ids)
        assert xi_ids.size == yi_ids.size == zi_ids.size
        sites1 = _itertools.izip(xi_ids.flat, yi_ids.flat, zi_ids.flat)

        xj_ids = _np.asarray(xj_ids)
        yj_ids = _np.asarray(yj_ids)
        zj_ids = _np.asarray(zj_ids)
        assert xj_ids.size == yj_ids.size == zj_ids.size

        if xj_ids.size:
            sites2 = _itertools.izip(xj_ids.flat, yj_ids.flat, zj_ids.flat)
            zipper = _itertools.product(sites1, sites2)
            size   = xi_ids.size * xj_ids.size
        else:
            zipper = _itertools.combinations(sites1, 2)
            size   = comb(xi_ids.size, 2, exact=1)

        distances = _np.zeros(size)
        for i, (p1, p2) in enumerate(zipper):
            dx_id = abs(p2[0] - p1[0])
            dy_id = abs(p2[1] - p1[1])
            dz_id = abs(p2[2] - p1[2])
            distances[i] = self.r1[dx_id, dy_id, dz_id]

        return distances

    def __str__(self):
        fmt = '%-15s ='
        s = _StringIO.StringIO()
        print >> s, '%s %s' % (fmt % 'mesh.shape', self.shape)
        print >> s, '%s %s' % (fmt % 'mesh.size', '(%s)' % self.size)
        print >> s, '%s %s' % (fmt % 'mesh.spacing', self.spacing)
        print >> s, '%s %s' % (fmt % 'mesh.origin', self.origin)
        print >> s, '%s %s' % (fmt % 'mesh.min', (
            self.x[0,0,0], self.y[0,0,0], self.z[0,0,0])
        )
        print >> s, '%s %s' % (fmt % 'mesh.max', (
            self.x[-1,0,0], self.y[0,-1,0], self.z[0,0,-1])
        )
        print >> s, '%s %s' % (fmt % 'mesh.length', (
            self.x[-1,0,0] - self.x[0,0,0],
            self.y[0,-1,0] - self.y[0,0,0],
            self.z[0,0,-1] - self.z[0,0,0])
        )
        return s.getvalue()

class Grid:
    def __init__(self):
        pass

    def init_vtk(self, start, delta, shape):
        x0, y0, z0 = start
        px, py, pz = shape
        dx, dy, dz = delta
        x1 = x0 + (px - 1) * dx
        y1 = y0 + (py - 1) * dy
        z1 = z0 + (pz - 1) * dz
        self.setup(x0, y0, z0, x1, y1, z1, px, py, pz)

    def init_linspace(self, start, stop, points):
        x0, y0, z0 = start
        x1, y1, z1 = stop
        px, py, pz = points
        self.setup(x0, y0, z0, x1, y1, z1, px, py, pz)

    def init_arange(self, start, stop, step):
        x0, y0, z0 = start
        x1, y1, z1 = stop
        dx, dy, dz = step
        px = int(abs(x1 - x0) / float(dx))
        py = int(abs(y1 - y0) / float(dy))
        pz = int(abs(z1 - z0) / float(dz))
        self.setup(x0, y0, z0, x1, y1, z1, px, py, pz)

    def setup(self, x0, y0, z0, x1, y1, z1, px, py, pz):

        assert px >= 1
        assert py >= 1
        assert pz >= 1

        assert x1 >= x0
        assert y1 >= y0
        assert z1 >= z0

        self.ox = None
        self.oy = None
        self.oz = None

        self.mx = None
        self.my = None
        self.mz = None

        self.mesh = None

        self.r0 = (x0, y0, z0)
        self.x0 = x0
        self.y0 = y0
        self.z0 = z0

        self.r1 = (x1, y1, z1)
        self.x1 = x1
        self.y1 = y1
        self.z1 = z1

        self.shape = (px, py, pz)
        self.px = px
        self.py = py
        self.pz = pz
        self.size = px * py * pz

        self.lx = abs(self.x1 - self.x0)
        self.ly = abs(self.y1 - self.y0)
        self.lz = abs(self.z1 - self.z0)
        self.volume = self.lx * self.ly * self.lz
        self.box = (self.lx, self.ly, self.lz)

        if (self.px - 1) > 0:
            self.dx = abs(self.x1 - self.x0) / float(self.px - 1)
        else:
            self.dx = 0

        if (self.py - 1) > 0:
            self.dy = abs(self.y1 - self.y0) / float(self.py - 1)
        else:
            self.dy = 0

        if (self.pz - 1) > 0:
            self.dz = abs(self.z1 - self.z0) / float(self.pz - 1)
        else:
            self.dz = 0

        self.spacing = (self.dx, self.dy, self.dz)
        self.dv = self.dx * self.dy * self.dz

        self.bounds = (self.x0, self.x1, self.y0, self.y1, self.z0, self.z1)
        self.extent = (0, self.px - 1, 0, self.py - 1, 0, self.pz - 1)

    def create_ogrid(self, force=False):
        if not force:
            if not self.ox is None:
                if not self.oy is None:
                    if not self.oz is None:
                        return self.ox, self.oy, self.oz
        self.ox = _np.linspace(self.x0, self.x1, self.px)
        self.oy = _np.linspace(self.y0, self.y1, self.py)
        self.oz = _np.linspace(self.z0, self.z1, self.pz)
        return self.ox, self.oy, self.oz

    def create_mgrid(self, force=False):
        if not force:
            if not self.mx is None:
                if not self.my is None:
                    if not self.mz is None:
                        return self.mx, self.my, self.mz
        self.mx, self.my, self.mz = \
            _np.mgrid[self.x0:self.x1:complex(self.px),
                      self.y0:self.y1:complex(self.py),
                      self.z0:self.z1:complex(self.pz)]
        return self.mx, self.my, self.mz

    def create_zeros(self):
        return _np.zeros(self.shape)

    def precalculate_mesh(self):
        self.mesh = Mesh(self)
        return self.mesh

    def __str__(self):
        s = _StringIO.StringIO()
        f = '  %-8s = %s'
        print >> s, '[Grid]'
        print >> s, ''
        print >> s, f % ('r0', self.r0)
        print >> s, f % ('x0', self.x0)
        print >> s, f % ('y0', self.y0)
        print >> s, f % ('z0', self.z0)
        print >> s, ''
        print >> s, f % ('r1', self.r1)
        print >> s, f % ('x1', self.x1)
        print >> s, f % ('y1', self.y1)
        print >> s, f % ('z1', self.z1)
        print >> s, ''
        print >> s, f % ('shape', self.shape)
        print >> s, f % ('px', self.px)
        print >> s, f % ('py', self.py)
        print >> s, f % ('pz', self.pz)
        print >> s, f % ('size', self.size)
        print >> s, ''
        print >> s, f % ('box', self.box)
        print >> s, f % ('lx', self.lx)
        print >> s, f % ('ly', self.ly)
        print >> s, f % ('lz', self.lz)
        print >> s, f % ('volume', self.volume)
        print >> s, ''
        print >> s, f % ('spacing', self.spacing)
        print >> s, f % ('dx', self.dx)
        print >> s, f % ('dy', self.dy)
        print >> s, f % ('dz', self.dz)
        print >> s, f % ('dv', self.dv)
        print >> s, ''
        print >> s, f % ('extent', self.extent)
        print >> s, f % ('bounds', self.bounds)
        return s.getvalue()

class XYZV:
    """
    Map sites onto a mesh
    """
    def __init__(self, grid, site_ids, site_values=None):

        imapper = IndexMapper(grid.shape)

        self.site_ids = _np.asarray(site_ids)

        if site_values is None:
            self.site_values = _np.ones(self.site_ids.size, dtype=float)

        elif _np.isscalar(site_values):
            self.site_values = _np.ones(self.site_ids.size, dtype=float)
            self.site_values = self.site_values * site_values

        else:
            self.site_values = _np.asarray(site_values)

        if not self.site_values.size == self.site_values.size:
            raise RuntimeError('site ids and values are not the same size')

        self.mesh_x, self.mesh_y, self.mesh_z = grid.create_mgrid()

        self.site_x_ids = imapper.indexX(self.site_ids)
        self.site_y_ids = imapper.indexY(self.site_ids)
        self.site_z_ids = imapper.indexZ(self.site_ids)

        self.x_values = self.mesh_x[self.site_x_ids,
                                    self.site_y_ids,
                                    self.site_z_ids]

        self.y_values = self.mesh_y[self.site_x_ids,
                                    self.site_y_ids,
                                    self.site_z_ids]

        self.z_values = self.mesh_z[self.site_x_ids,
                                    self.site_y_ids,
                                    self.site_z_ids]

        self.mesh_v = grid.create_zeros()

        self.mesh_v[self.site_x_ids,
                    self.site_y_ids,
                    self.site_z_ids] = self.site_values

        self.xyz = _np.dstack([self.x_values,
                               self.y_values,
                               self.z_values]).flatten()

def grid_from_checkpoint(chk, spacing=(1, 1, 1), origin=(0.5, 0.5, 0.5)):
    """
    Alternate constructor for grid.  Creates a Grid from a checkpoint file

    chk     : checkpoint file object, file name, or file handle
    spacing : 3-tuple of grid spacing
    origin  : 3-tuple of grid origin
    """
    if not isinstance(chk, langmuir.checkpoint.CheckPoint):
        chk = langmuir.checkpoint.load(chk)

    if chk.parameters.has_key('grid.x'):
        grid_x = int(chk.parameters['grid.x'])
    else:
        print 'warning: grid.x not found in checkpoint file (assuming 1)'
        grid_x = 1

    if chk.parameters.has_key('grid.y'):
        grid_y = int(chk.parameters['grid.y'])
    else:
        print 'warning: grid.y not found in checkpoint file (assuming 1)'
        grid_y = 1

    if chk.parameters.has_key('grid.z'):
        grid_z = int(chk.parameters['grid.z'])
    else:
        print 'warning: grid.z not found in checkpoint file (assuming 1)'
        grid_z = 1

    shape = (grid_x, grid_y, grid_z)
    print shape

    grid = Grid()
    grid.init_vtk(origin, spacing, shape)

    return grid