# -*- coding: utf-8 -*-
import matplotlib.pyplot as _plt
import numpy as _np

class WaveDimensions:
    """Compute wavelength, wavenumber, etc from an interval length (L)
    and number of waves (n).

    :param L: interval length
    :param n: number of waves in interval

    :type L: float
    :type n: int

    >>> wx = WaveDimensions(10, 2)
    >>> print wx
    [Wave Dimensions]
        L      = 10
        n      = 2
        lambda = 5.00000e+00
        nubar  = 2.00000e-01
        k      = 1.25664e+00
    """
    def __init__(self, L=2 * _np.pi, n=1):
        self.L = L
        self.n = n
        self.calc()

    def calc(self, L=None, n=None):
        """Perform calculations to compute wavelength, wavenumber, etc.
        Called automatically in constructor.

        :param L: interval length
        :param n: number of waves in interval

        :type L: float
        :type n: int
        """
        if not L is None:
            self.L = L
        if not n is None:
            self.n = n
        self.wavelength = float(self.L) / self.n
        self.nubar = 1 / self.wavelength
        self.k = (2 * _np.pi) / self.wavelength

    def __str__(self):
        s  = '[Wave Dimensions]\n'
        s += '    L      = %d\n' % self.L
        s += '    n      = %d\n' % self.n
        s += '    lambda = %.5e\n' % self.wavelength
        s += '    nubar  = %.5e\n' % self.nubar
        s += '    k      = %.5e\n' % self.k
        return s

def gyroid(x, y, z, kx, ky, kz):
    """
    Surface function f(x,y,z) for gyroid

    :param x: x-value(s)
    :param y: y-value(s)
    :param z: z-value(s)
    :param kx: 2 pi nx / Lx
    :param ky: 2 pi ny / Ly
    :param kz: 2 pi nz / Lz

    :type x: float
    :type y: float
    :type z: float
    :type kx: float
    :type ky: float
    :type kz: float

    >>> w = WaveDimensions(10, 2)
    >>> x, y, z = np.mgrid[0:10:100j,0:10:100j,0:10:100j]
    >>> v = gyroid(x, y, v, w.k, w.k, w.k)
    """
    value = _np.cos(kx * x) * _np.sin(ky * y) + \
            _np.cos(ky * y) * _np.sin(kz * z) + \
            _np.cos(kz * z) * _np.sin(kx * x)
    return value

def scherk_first_surface(x, y, z, kx, ky, kz):
    """
    Surface function f(x,y,z) for scherk

    :param x: x-value(s)
    :param y: y-value(s)
    :param z: z-value(s)
    :param kx: 2 pi nx / Lx
    :param ky: 2 pi ny / Ly
    :param kz: 2 pi nz / Lz

    :type x: float
    :type y: float
    :type z: float
    :type kx: float
    :type ky: float
    :type kz: float

    >>> w = WaveDimensions(10, 2)
    >>> x, y, z = np.mgrid[0:10:100j,0:10:100j,0:10:100j]
    >>> v = scherk_first_surface(x, y, v, w.k, w.k, w.k)
    """
    value = _np.exp(kx / (2.0 * _np.pi) * x) * _np.cos(ky * y) - _np.cos(kz * z)
    return value

def schwarz_p_surface(x, y, z, kx, ky, kz):
    """
    Surface function f(x,y,z) for psurface

    :param x: x-value(s)
    :param y: y-value(s)
    :param z: z-value(s)
    :param kx: 2 pi nx / Lx
    :param ky: 2 pi ny / Ly
    :param kz: 2 pi nz / Lz

    :type x: float
    :type y: float
    :type z: float
    :type kx: float
    :type ky: float
    :type kz: float

    >>> w = WaveDimensions(10, 2)
    >>> x, y, z = np.mgrid[0:10:100j,0:10:100j,0:10:100j]
    >>> v = schwarz_p_surface(x, y, v, w.k, w.k, w.k)
    """
    value = _np.cos(kx * x) + _np.cos(ky * y) + _np.cos(kz * z)
    return value

def schwarz_d_surface(x, y, z, kx, ky, kz):
    """
    Surface function f(x,y,z) for dsurface

    :param x: x-value(s)
    :param y: y-value(s)
    :param z: z-value(s)
    :param kx: 2 pi nx / Lx
    :param ky: 2 pi ny / Ly
    :param kz: 2 pi nz / Lz

    :type x: float
    :type y: float
    :type z: float
    :type kx: float
    :type ky: float
    :type kz: float

    >>> w = WaveDimensions(10, 2)
    >>> x, y, z = np.mgrid[0:10:100j,0:10:100j,0:10:100j]
    >>> v = schwarz_d_surface(x, y, v, w.k, w.k, w.k)
    """
    value = _np.sin(kx * x) * _np.sin(ky * y) * _np.sin(kz * z) + \
            _np.sin(kx * x) * _np.cos(ky * y) * _np.cos(kz * z) + \
            _np.cos(kx * x) * _np.sin(ky * y) * _np.cos(kz * z) + \
            _np.cos(kx * x) * _np.cos(ky * y) * _np.sin(kz * z)
    return value

def band3D(x, y, z, kx, ky, kz):
    """
    Surface function f(x,y,z) for bands

    :param x: x-value(s)
    :param y: y-value(s)
    :param z: z-value(s)
    :param kx: 2 pi nx / Lx
    :param ky: 2 pi ny / Ly
    :param kz: 2 pi nz / Lz

    :type x: float
    :type y: float
    :type z: float
    :type kx: float
    :type ky: float
    :type kz: float

    >>> w = WaveDimensions(10, 2)
    >>> x, y, z = np.mgrid[0:10:100j,0:10:100j,0:10:100j]
    >>> v = band3D(x, y, v, w.k, w.k, w.k)
    """
    xsize, ysize, zsize = x.shape
    dy = y[0,1,0] - y[0,0,0]
    dz = z[0,0,1] - z[0,0,0]
    dy = int((2 + 1e-8) * _np.pi / ( ky * dy ))
    dz = int((2 + 1e-8) * _np.pi / ( kz * dz ))
    value = _np.ones(x.shape) * -0.1
    xsize, ysize, zsize = x.shape
    paint_checkerboard_yz(value, dy, dz)
    return value

def paint_cube(data, x, dx, y, dy, z, dz, value=0.1):
    """Draw a cube of traps

    The cube has a (length, width, height) of (dx, dy, dz) and
    lower back left cornor of (x, y, z)

    :param x: the lower front left cornor x-value
    :param y: the lower front left cornor y-value
    :param z: the lower front left cornor z-value
    :param dx: the width of the plane
    :param dy: the height of the plane
    :param dz: the height of the plane
    :param value: trap energy value in eV

    :type x: int
    :type y: int
    :type z: int
    :type dx: int
    :type dy: int
    :type dz: int
    :type value: float

    >>> data = np.zeros((64, 64, 64))
    >>> data = paint_cube(data, 0, 16, 0, 16, 0, 16, 1.0)
    """
    print x, x + dx, y, y + dy, z, z + dz
    data[x:x+dx,y:y+dy,z:z+dz] = value
    return data

def paint_square_xy(data, x, dx, y, dy, z=0, value=0.1):
    """Draw a square of traps in the xy plane

    The plane is parallel to the xy-plane, has a thickness 1,
    cornor at (x, y), and (width,height) of (dx, dy).

    :param x: the lower left cornor x-value
    :param dx: the width of the plane
    :param y: the lower left cornor y-value
    :param dy: the height of the plane
    :param value: trap energy value in eV

    :type x: int
    :type y: int
    :type z: int
    :type dx: int
    :type dy: int
    :type value: float

    >>> data = np.zeros((64, 64, 64))
    >>> data = paint_square_xy(data, 0, 16, 0, 16, 0, 1.0)
    """
    data[x:x+dx,y:y+dy,z] = value
    return data

def paint_square_xz(data, x, dx, z, dz, y=0, value=0.1):
    """Draw a square of traps in the xy plane

    The plane is parallel to the xz-plane, has a thickness 1,
    cornor at (x, z), and (width,height) of (dx, dz).

    :param x: the lower left cornor x-value
    :param dx: the width of the plane
    :param z: the lower left cornor z-value
    :param dz: the height of the plane
    :param value: trap energy value in eV

    :type x: int
    :type y: int
    :type z: int
    :type dx: int
    :type dz: int
    :type value: float

    >>> data = np.zeros((64, 64, 64))
    >>> data = paint_square_xz(data, 0, 16, 0, 16, 0, 1.0)
    """
    data[x:x+dx,y,z:z+dz] = value
    return data

def paint_square_yz(data, y, dy, z, dz, x=0, value=0.1):
    """Draw a square of traps in the xy plane

    The plane is parallel to the yz-plane, has a thickness 1,
    cornor at (y, z), and (width,height) of (dy, dz).

    :param y: the lower left cornor y-value
    :param dy: the height of the plane
    :param z: the lower left cornor z-value
    :param dz: the width of the plane
    :param value: trap energy value in eV

    :type x: int
    :type y: int
    :type z: int
    :type dy: int
    :type dz: int
    :type value: float

    >>> data = np.zeros((64, 64, 64))
    >>> data = paint_square_yz(data, 0, 16, 0, 16, 0, 1.0)
    """
    data[x,y:y+dy,z:z+dz] = value
    return data

def paint_plane_xy(data, z, dz, value=1.0):
    """Draw an xy plane of traps

    The plane is parallel to the xy-plane and has a thickness dz
    and is located at z.

    :param z: distance of plane from the zero xy-plane
    :param dz: thickness of plane
    :param value: trap energy value in eV

    :type z: int
    :type dz: int
    :type value: float

    >>> data = np.zeros((64, 64, 64))
    >>> data = paint_plane_xy(data, 0, 8, 1.0)
    """
    data[:,:,z:z+dz] = value
    return data

def paint_plane_yz(data, x, dx, value=1.0):
    """Draw an yz plane of traps

    The plane is parallel to the yz-plane and has a thickness dx
    and is located at x.

    :param x: distance of plane from the zero yz-plane
    :param dx: thickness of plane
    :param value: trap energy value in eV

    :type x: int
    :type dx: int
    :type value: float

    >>> data = np.zeros((64, 64, 64))
    >>> data = paint_plane_yz(data, 0, 8, 1.0)
    """
    data[x:x+dx,:,:] = value
    return data

def paint_plane_xz(data, y, dy, value=1.0):
    """Draw an xz plane of traps

    The plane is parallel to the xz-plane and has a thickness dy
    and is located at y.

    :param y: distance of plane from the zero xz-plane
    :param dy: thickness of plane
    :param value: trap energy value in eV

    :type y: int
    :type dy: int
    :type value: float

    >>> data = np.zeros((64, 64, 64))
    >>> data = paint_plane_xz(data, 0, 8, 1.0)
    """
    data[y:y+dy,:,:] = value
    return data

def paint_stripe_dx(data, dx, value=1.0):
    """Stack slabs along the x-direction of thickness dx.

    Draw alternating rectangular prisms with of thickness dx forall x, y

    :param dx: thickness of slab
    :param value: trap energy value in eV

    :type dx: int
    :type value: float

    >>> data = np.zeros((64, 64, 64))
    >>> data = paint_stripe_dx(data, 8, 1.0)
    """
    xsize, ysize, zsize = data.shape
    for xi in range(0, ysize, 2*dx):
        data[xi:xi+dx,:,:] = value
    return data

def paint_stripe_dy(data, dy, value=1.0):
    """Stack slabs along the y-direction of thickness dy.

    Draw alternating rectangular prisms with of thickness dy forall x, z

    :param dy: thickness of slab
    :param value: trap energy value in eV

    :type dy: int
    :type value: float

    >>> data = np.zeros((64, 64, 64))
    >>> data = paint_stripe_dy(data, 8, 1.0)
    """
    xsize, ysize, zsize = data.shape
    for yi in range(0, ysize, 2*dy):
        data[:,yi:yi+dy,:] = value
    return data

def paint_stripe_dz(data, dz, value=1.0):
    """Stack slabs along the z-direction of thickness dz.

    Draw alternating rectangular prisms with of thickness dz forall x, y

    :param dz: thickness of slab
    :param value: trap energy value in eV

    :type dz: int
    :type value: float

    >>> data = np.zeros((64, 64, 64))
    >>> data = paint_stripe_dz(data, 8, 1.0)
    """
    xsize, ysize, zsize = data.shape
    for zi in range(0, ysize, 2*dz):
        data[:,:,zi:zi+dz] = value
    return data

def paint_checkerboard_xy(data, dx, dy, value=1.0):
    """Extend a checkerboard pattern along the z-direction.

    Draw a checkerboard pattern in xy plane, for all z values

    :param dx: x-size of checkerboard square
    :param dy: y-size of checkerboard square
    :param value: trap energy value in eV

    :type dx: int
    :type dy: int
    :type value: float

    >>> data = np.zeros((64, 64, 64))
    >>> data = paint_checkerboard_xy(data, 8, 8, 1.0)
    """
    xsize, ysize, zsize = data.shape
    for xi in range(0, xsize, 2*dx):
        for yi in range(0, ysize, 2*dy):
            data[xi:xi+dx,yi:yi+dy,:] = value
    for xi in range(dx, xsize, 2*dx):
        for yi in range(dy, ysize, 2*dy):
            data[xi:xi+dx,yi:yi+dy,:] = value
    return data

def paint_checkerboard_yz(data, dy, dz, value=1.0):
    """Extend a checkerboard pattern along the x-direction.

    Draw a checkerboard pattern in yz plane, for all x values

    :param dy: y-size of checkerboard square
    :param dz: z-size of checkerboard square
    :param value: trap energy value in eV

    :type dy: int
    :type dz: int
    :type value: float

    >>> data = np.zeros((64, 64, 64))
    >>> data = paint_checkerboard_yz(data, 8, 8, 1.0)
    """
    xsize, ysize, zsize = data.shape
    for yi in range(0, ysize, 2*dy):
        for zi in range(0, zsize, 2*dz):
            data[:,yi:yi+dy,zi:zi+dz] = value
    for yi in range(dy, ysize, 2*dy):
        for zi in range(dz, zsize, 2*dz):
            data[:,yi:yi+dy,zi:zi+dz] = value
    return data

def show3D(x, y, z, v, show=False, **kwargs):
    """
    Wrapper around mayavi contour3D (slow) to visualize surface.

    :param x: set of x-points
    :param y: set of y-points
    :param z: set of z-points
    :param v: set of v-points (surface values)
    :param show: open mayavi window

    :type x: float
    :type y: float
    :type z: float
    :type v: float
    :type show: bool

    >>> w = WaveDimensions(10, 2)
    >>> x, y, z = np.mgrid[0:10:100j,0:10:100j,0:10:100j]
    >>> v = gyroid(x, y, v, w.k, w.k, w.k)
    >>> show3D(x, y, z, v)
    >>> mlab.show()
    """
    import mayavi.mlab as mlab
    if not kwargs.has_key('contours'):
        kwargs['contours'] = 3
    obj = mlab.contour3d(x, y, z, v, **kwargs)
    mlab.outline(obj, color=(0, 0, 0))
    if show:
        mlab.show()

def showXY(x, y, z, v, zlevel=0, *args, **kwargs):
    """
    Wrapper around pyplot.contourf to visualize surface slice.

    :param x: set of x-points
    :param y: set of y-points
    :param z: set of z-points
    :param v: set of v-points (surface values)
    :param zlevel: z-value of slice

    :type x: float
    :type y: float
    :type z: float
    :type v: float
    :type zlevel: int

    >>> w = WaveDimensions(10, 2)
    >>> x, y, z = np.mgrid[0:10:100j,0:10:100j,0:10:100j]
    >>> v = gyroid(x, y, v, w.k, w.k, w.k)
    >>> showXY(x, y, z, v)
    >>> plt.show()
    """
    _plt.contourf(x[:,:,zlevel], y[:,:,zlevel], v[:,:,zlevel], *args,
                 **kwargs)
