# -*- coding: utf-8 -*-
"""
.. note::
    Functions for calculating surfaces and analyzing them.

.. moduleauthor:: Adam Gagorik <adam.gagorik@gmail.com>
"""
import langmuir as lm
import numpy as np

try:
    import scipy.special as special
    import scipy.fftpack as fftpack
    import scipy.ndimage as ndimage
    import scipy.signal as signal
    import scipy.stats as stats
    import scipy.misc as misc
except ImportError:
    pass

def make_3D(array):
    """
    Force the numpy array passed to be 3D via :py:func:`np.expand_dims`.

    :param array: numpy array
    :type array: :py:class:`numpy.ndarray`
    """
    ndims = len(array.shape)
    if ndims >= 3:
        return array

    if ndims == 2:
        return np.expand_dims(array, 2)

    if ndims == 1:
        return np.expand_dims(np.expand_dims(array, 1), 1)

    raise RuntimeError, 'can not expand dimensions'

def load_ascii(handle, square=False, cube=False, shape=None, **kwargs):
    """
    Wrapper around np.loadtxt.  Forces data to be at least 3 dimensions.

    :param square: reshape as if data is NxN
    :param cube: reshape as if data is NxNxN
    :parm shape: reshape data

    :param square: bool
    :param cube: bool
    :param shape: list
    """
    # load the data
    image = np.loadtxt(handle, **kwargs)

    if square:
        try:
            size = int(np.sqrt(image.size))
            image = np.reshape(image, (size, size))
        except ValueError:
            raise RuntimeError, 'can not reshape data'

    if cube:
        try:
            size = int(special.cbrt(image.size))
            image = np.reshape(image, (size, size, size))
        except ValueError:
            raise RuntimeError, 'can not reshape data'

    if not shape is None:
        image = np.reshape(image, shape)

    # force data to be 3D
    return make_3D(image)

def load(handle, rot90=-1, **kwargs):
    """
    Load surface from file.  Takes into account the file extension.

    ===== ===============================
    *ext* *func*
    ===== ===============================
    pkl   :py:meth:`common.load_pkl`
    npy   :py:func:`numpy.load`
    csv   :py:meth:`surface.load_ascii`
    txt   :py:meth:`surface.load_ascii`
    dat   :py:meth:`surface.load_ascii`
    png   :py:meth:`scipy.ndimage.imread`
    jpg   :py:meth:`scipy.ndimage.imread`
    jpeg  :py:meth:`scipy.ndimage.imread`
    ===== ===============================

    :param handle: filename
    :param rot90: number of times to rotate image by 90 degrees

    :type handle: str
    :type rot90: int

    :return: image
    :rtype: :py:class:`numpy.ndarray`

    .. warning::
        image file (png, jpg, etc) data is forced into range [0,255].

    .. warning::
        data is always made 3D via :py:func:`numpy.expand_dims`

    .. warning::
        image data is rotated by -90 degrees.
    """
    stub, ext = lm.common.splitext(handle)

    if ext == '.pkl':
        return make_3D(lm.common.load_pkl(handle, **kwargs))

    if ext == '.npy':
        return make_3D(np.load(handle, **kwargs))

    if ext in ['.csv', '.txt', '.dat']:
        return lm.surface.load_ascii(handle, **kwargs)

    if ext in ['.png', '.jpg', '.jpeg']:
        _kwargs = dict(flatten=True)
        _kwargs.update(**kwargs)
        image = ndimage.imread(handle, **_kwargs)
        image = np.rot90(image, rot90)
        return make_3D(image)

    raise RuntimeError, 'ext=%s not supported' % ext

def save_vti(handle, array, **kwargs):
    """
    Save numpy array to vtkImageData XML file.  You can open it in paraview.

    :param handle: filename
    :param array: data

    :type handle: str
    :type array: :py:class:`numpy.ndarray`
    """
    vtkImageData = lm.vtkutils.create_image_data_from_array(array, **kwargs)
    lm.vtkutils.save_image_data(handle, vtkImageData)

def save(handle, obj, zlevel=0, **kwargs):
    """
    Save object to a file.  Takes into account the file extension.

    ===== ===================================
    *ext* *func*
    ===== ===================================
    pkl   :py:meth:`common.load_pkl`
    npy   :py:func:`numpy.load`
    vti   :py:meth:`vtkutils.save_image_data`
    csv   :py:meth:`surface.load_ascii`
    txt   :py:meth:`surface.load_ascii`
    dat   :py:meth:`surface.load_ascii`
    png   :py:meth:`scipy.misc.imsave`
    jpg   :py:meth:`scipy.misc.imsave`
    jpeg  :py:meth:`scipy.misc.imsave`
    ===== ===================================

    :param handle: filename
    :param obj: object to save
    :param zlevel: slice z-index

    :type handle: str
    :type array: :py:class:`numpy.ndarray`
    :type zlevel: int

    .. warning::
        image file (png, jpg, etc) data is forced into range [0,255].

    .. warning::
        if ndims is 3 and an image file is being saved, only a slice is saved.
    """
    stub, ext = lm.common.splitext(handle)

    if ext == '.pkl':
        lm.common.save_pkl(obj, handle, **kwargs)
        return handle

    if ext == '.npy':
        np.save(handle, obj, **kwargs)
        return handle

    if ext == '.vti':
        lm.surface.save_vti(handle, obj, **kwargs)
        return handle

    if ext in ['.csv', '.txt', '.dat']:
        _kwargs = dict(fmt='%+.18e', sep=',')
        _kwargs.update(**kwargs)
        try:
            np.savetxt(handle, obj, **_kwargs)
        except TypeError:
            handle = lm.common.zhandle(handle, 'wb')
            print >> handle, '# ' + ' '.join([str(s) for s in obj.shape])
            obj = np.reshape(obj, obj.size)
            np.savetxt(handle, obj, **_kwargs)
        return handle

    if ext in ['.png', '.jpg', '.jpeg']:
        if len(obj.shape) > 2:
            obj = obj[:,:,zlevel]
        misc.imsave(handle, obj)
        return handle

    raise RuntimeError, 'ext=%s not supported' % ext

def threshold(a, v=0, v0=0, v1=1, copy=False):
    """
    Set values in array above {v} to {v1}, and below {v} to {v0}.

    :param v: threshold value
    :param v0: lower value
    :param v1: upper value
    :param copy: copy array

    :type v: float
    :type v0: float
    :type v1: float
    :type copy: bool
    """
    if copy:
        t = np.copy(a)
    else:
        t = a
    t[t <= v] = 0
    t[t >  0] = 1
    if not v0 == 0 or not v1 == 1:
        s0 = [t == 0]
        s1 = [t == 1]
        if not v0 == 0:
            t[s0] = v0
        if not v1 == 1:
            t[s1] = v1
    return t

def linear_mapping(array, n=0.0, m=1.0):
    """
    Map values in array to fall in the range [n,m].

    :param array: array like object
    :param n: lower bound
    :param m: upper bound

    :type array: list
    :type n: float
    :type m: float
    """
    array = np.asanyarray(array)
    a = np.amin(array)
    b = np.amax(array)
    return (array - a) / (b - a) * (m - n) + n

def rfunc(size=None):
    """
    Produces numbers in the range [-0.5, 0.5].

    :param size: shape of output
    :type: int
    """
    return np.random.random(size) - 0.5

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
    def __init__(self, L=2 * np.pi, n=1):
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
        self.k = (2 * np.pi) / self.wavelength

    def __str__(self):
        s  = '[Wave Dimensions]\n'
        s += '    L      = %d\n' % self.L
        s += '    n      = %d\n' % self.n
        s += '    lambda = %.5e\n' % self.wavelength
        s += '    nubar  = %.5e\n' % self.nubar
        s += '    k      = %.5e\n' % self.k
        return s

f_sin_x = lambda x, y, z, kx, ky, kz : np.sin(kx * x)
f_sin_y = lambda x, y, z, kx, ky, kz : np.sin(ky * y)
f_sin_z = lambda x, y, z, kx, ky, kz : np.sin(kz * z)
f_cos_x = lambda x, y, z, kx, ky, kz : np.cos(kx * x)
f_cos_y = lambda x, y, z, kx, ky, kz : np.cos(ky * y)
f_cos_z = lambda x, y, z, kx, ky, kz : np.cos(kz * z)

sin_x = lambda x, y, z, wx, wy, wz : np.sin(wx.k * x)
sin_y = lambda x, y, z, wx, wy, wz : np.sin(wy.k * y)
sin_z = lambda x, y, z, wx, wy, wz : np.sin(wz.k * z)
cos_x = lambda x, y, z, wx, wy, wz : np.cos(wx.k * x)
cos_y = lambda x, y, z, wx, wy, wz : np.cos(wy.k * y)
cos_z = lambda x, y, z, wx, wy, wz : np.cos(wz.k * z)

def f_gyroid(x, y, z, kx, ky, kz):
    """
    Surface function f(x,y,z) for gyroid.

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
    >>> gyroid(x, y, z, w.k, w.k, w.k)
    """
    value = np.cos(kx * x) * np.sin(ky * y) + \
            np.cos(ky * y) * np.sin(kz * z) + \
            np.cos(kz * z) * np.sin(kx * x)
    return value

def gyroid(x, y, z, wx, wy, wz):
    """
    Wrapper around f_* that uses WaveDimensions.
    """
    return f_gyroid(x, y, z, wx.k, wy.k, wz.k)

def f_scherk_first_surface(x, y, z, kx, ky, kz):
    """
    Surface function f(x,y,z) for scherk.

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
    >>> scherk_first_surface(x, y, z, w.k, w.k, w.k)
    """
    value = np.exp(kx / (2.0 * np.pi) * x) * np.cos(ky * y) - np.cos(kz * z)
    return value

def scherk_first_surface(x, y, z, wx, wy, wz):
    """
    Wrapper around f_* that uses WaveDimensions.
    """
    return f_scherk_first_surface(x, y, z, wx.k, wy.k, wz.k)

def f_schwarz_p_surface(x, y, z, kx, ky, kz):
    """
    Surface function f(x,y,z) for psurface.

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
    >>> schwarz_p_surface(x, y, z, w.k, w.k, w.k)
    """
    value = np.cos(kx * x) + np.cos(ky * y) + np.cos(kz * z)
    return value

def schwarz_p_surface(x, y, z, wx, wy, wz):
    """
    Wrapper around f_* that uses WaveDimensions.
    """
    return f_schwarz_p_surface(x, y, z, wx.k, wy.k, wz.k)

def f_schwarz_d_surface(x, y, z, kx, ky, kz):
    """
    Surface function f(x,y,z) for dsurface.

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
    >>> schwarz_d_surface(x, y, z, w.k, w.k, w.k)
    """
    value = np.sin(kx * x) * np.sin(ky * y) * np.sin(kz * z) + \
            np.sin(kx * x) * np.cos(ky * y) * np.cos(kz * z) + \
            np.cos(kx * x) * np.sin(ky * y) * np.cos(kz * z) + \
            np.cos(kx * x) * np.cos(ky * y) * np.sin(kz * z)
    return value

def schwarz_d_surface(x, y, z, wx, wy, wz):
    """
    Wrapper around f_* that uses WaveDimensions.
    """
    return f_schwarz_d_surface(x, y, z, wx.k, wy.k, wz.k)

def f_bandXY(x, y, z, kx, ky, kz):
    """
    Surface function f(x,y,z) for bands that run along z-direction.

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
    >>> bandXY(x, y, z, w.k, w.k, w.k)
    """
    return np.sin(kx * x) * np.sin(ky * y)

def bandXY(x, y, z, wx, wy, wz):
    """
    Wrapper around f_* that uses WaveDimensions.
    """
    return f_bandXY(x, y, z, wx.k, wy.k, wz.k)

def f_bandXZ(x, y, z, kx, ky, kz):
    """
    Surface function f(x,y,z) for bands that run along y-direction.

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
    >>> bandXZ(x, y, z, w.k, w.k, w.k)
    """
    return np.sin(kx * x) * np.sin(kz * z)

def bandXZ(x, y, z, wx, wy, wz):
    """
    Wrapper around f_* that uses WaveDimensions.
    """
    return f_bandXZ(x, y, z, wx.k, wy.k, wz.k)

def f_bandYZ(x, y, z, kx, ky, kz):
    """
    Surface function f(x,y,z) for bands that run along x-direction.

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
    >>> bandYZ(x, y, z, w.k, w.k, w.k)
    """
    return np.sin(ky * y) * np.sin(kz * z)

def bandYZ(x, y, z, wx, wy, wz):
    """
    Wrapper around f_* that uses WaveDimensions.
    """
    return f_bandYZ(x, y, z, wx.k, wy.k, wz.k)

class Kernel(object):
    """
    An x, y, z, v mgrid.

    :param xmin: lower x
    :param xmax: upper x
    :param ymin: lower y
    :param ymax: upper y
    :param zmin: lower z
    :param zmax: upper z
    :param spacing: grid spacing

    :type xmin: float
    :type xmax: float
    :type ymin: float
    :type ymax: float
    :type zmin: float
    :type zmax: float
    :type spacing: float
    """

    def __init__(self, xmin, xmax, ymin, ymax, zmin, zmax, spacing=1.0):
        self.x, self.y, self.z = np.mgrid[xmin:xmax+1e-10:spacing,
                                          ymin:ymax+1e-10:spacing,
                                          zmin:zmax+1e-10:spacing]
        self.v = np.zeros(self.x.shape)
        self.spacing = spacing

class FuncKernel(Kernel):
    """
    An x, y, z, v mgrid.  Computes v using the function = f(x, y, z) passed.
    See Kernel for more parameters.
    """
    def __init__(self, func, *args, **kwargs):
        Kernel.__init__(self, *args, **kwargs)
        self.v = func(self.x, self.y, self.z)

class SimpleKernel(FuncKernel):
    """
    An x, y, z, v mgrid.  Computes v using the function = f(x, y, z) passed.
    Creates x, y, z domain using spacing and number of points.

    :param func: function of x, y, z
    :param nx: x-direction has 2*nx + 1 points
    :param ny: y-direction has 2*ny + 1 points
    :param nz: z-direction has 2*nz + 1 points
    :param spacing: grid spacing

    :type func: func
    :type nx: int
    :type ny: int
    :type nz: int
    :type spacing: double
    """
    def __init__(self, func, nx=3, ny=3, nz=3, spacing=1.0):
        xmin, xmax = -spacing * nx, spacing * nx
        ymin, ymax = -spacing * ny, spacing * ny
        zmin, zmax = -spacing * nz, spacing * nz
        args = [func, xmin, xmax, ymin, ymax, zmin, zmax, spacing]
        FuncKernel.__init__(self, *args)

class RandomKernel(SimpleKernel):
    """
    An x, y, z, v mgrid.  Computes v using random noise. Creates x, y, z domain
    using spacing and number of points.  See SimpleKernel for more parameters.
    """
    def __init__(self, *args, **kwargs):
        func = lambda x, y, z : np.random.random(x.shape)
        SimpleKernel.__init__(self, func, *args, **kwargs)
        self.v = self.v / np.sum(self.v)

class GaussianKernel(Kernel):
    """
    An x, y, z, v mgrid.  The size of the grid is determined using the stdev
    of the Gaussian PDF.

    :param sx: sigma x
    :param sy: sigma y
    :param sz: sigma z
    :param mx: mean x
    :param my: mean y
    :param mz: mean z
    :param spacing: grid spacing

    :type sx: float
    :type sy: float
    :type sz: float
    :type mx: float
    :type my: float
    :type mz: float
    :type spacing: float
    """

    def __init__(self, sx, sy, sz, mx=0.0, my=0.0, mz=0.0, spacing=1.0):
        self.sx, self.sy, self.sz = sx, sy, sz
        xmin, xmax = -3.0*self.sx, 3.0*self.sx
        ymin, ymax = -3.0*self.sy, 3.0*self.sy
        zmin, zmax = -3.0*self.sz, 3.0*self.sz
        Kernel.__init__(self, xmin, xmax, ymin, ymax, zmin, zmax, spacing)
        dst_x  = stats.norm(mx, sx)
        dst_y  = stats.norm(my, sy)
        dst_z  = stats.norm(mz, sz)
        pdf_x  = dst_x.pdf
        pdf_y  = dst_y.pdf
        pdf_z  = dst_z.pdf
        self.v = pdf_x(self.x) * pdf_y(self.y) * pdf_z(self.z)
        self.v = self.v / np.sum(self.v)

    def __str__(self):
        s = '[Kernel]\n'
        s = s + '  %-7s = %s\n' % ('shape', self.v.shape)
        s = s + '  %-7s = %s\n' % ('spacing', self.spacing)
        return s

class Isotropic(object):
    """
    Performs convolution of random noise with a kernel to make morphology.

    :param xsize: x grid points
    :param ysize: y grid points
    :param zsize: z grid points
    :param kernel: Kernel instance
    :param rfunc: function that produces random numbers in range [-0.5,0.5]

    :type xsize: int
    :type ysize: int
    :type zsize: int
    :type kernel: Kernel
    :type rfunc: func
    """

    def __init__(self, grid, kernel, rfunc=rfunc, v=0.0, mode='same',
                 verbose=False):
        if verbose: print 'isotropic: be patient, convolution is slow'
        self.kernel = kernel
        self.grid   = grid

        if verbose: print 'isotropic: creating noise'
        self.noise  = rfunc(self.grid.shape)

        #kernel and grid must have the same spacing
        self.factor = self.grid.dx/self.kernel.spacing
        assert self.factor >= 1.0
        if verbose: print 'isotropic: factor = %.3e' % self.factor

        if self.factor > 1.0:
            # calculate on grid with same spacing as kernel
            self.z_grid = lm.grid.Grid(grid.lx, grid.ly, grid.lz)
            self.z_grid.refine(self.factor)

            # zoom in random noise
            if verbose: print 'isotropic: interpolating noise'
            self.z_noise = ndimage.zoom(self.noise , zoom=self.factor)

            # convolve kernel and noise
            if verbose: print 'isotropic: convoluting'
            self.z_image = signal.convolve(self.z_noise, self.kernel.v,
                mode=mode)

            # zoom out on image
            if verbose: print 'isotropic: interpolating image'
            self.image = ndimage.zoom(self.z_image , zoom=1.0/self.factor)
        else:
            # no zooming needed
            self.z_grid  = self.grid
            self.z_noise = self.noise
            if verbose: print 'isotropic: convoluting'
            self.z_image = signal.convolve(self.z_noise, self.kernel.v,
                mode=mode)
            self.image = self.z_image

        if verbose: print ''
        #print 'grid   == z_grid  ', self.grid is self.z_grid
        #print 'noise  == z_noise ', self.noise is self.z_noise
        #print 'image  == z_image ', self.image is self.z_image
        #print 'thresh == z_thresh', self.thresh is self.z_thresh

    def __str__(self):
        s = '[Isotropic]\n'
        s = s + '  %-6s = %s, %s\n' % ('grid', self.grid.shape,
            self.z_grid.shape  )
        s = s + '  %-6s = %s, %s\n' % ('noise', self.noise.shape,
            self.z_noise.shape )
        s = s + '  %-6s = %s, %s\n' % ('image', self.image.shape,
            self.z_image.shape )
        return s

def f_isotropic(x, y, z, sx, sy, sz, full=False):
    """
    Surface function f(x,y,z) for isotropic morphology according to Jake.

    :param x: x-value(s)
    :param y: y-value(s)
    :param z: z-value(s)
    :param sx: sigma x
    :param sy: sigma y
    :param sz: sigma z

    :type x: float
    :type y: float
    :type z: float
    :type sx: float
    :type sy: float
    :type sz: float

    >>> x, y, z = np.mgrid[0:10:100j,0:10:100j,0:10:100j]
    >>> isotropic(x, y, z, 1, 1, 1)
    """
    xsize = int(x.shape[0])
    ysize = int(x.shape[1])
    zsize = int(x.shape[2])
    k = GaussianKernel(sx, sy, sz, spacing=1.0)
    g = lm.grid.Grid(xsize, ysize, zsize)
    i = Isotropic(g, k)
    if full:
        return i
    return i.image

def isotropic(x, y, z, wx, wy, wz, full=True):
    return f_isotropic(x, y, z, wx.wavelength, wx.wavelength, wx.wavelength,
                       full)

class FFT(object):
    def __init__(self):
        pass

    def _delta(self, a, s0, s1):
        a = np.asanyarray(a)
        try:
            delta = abs(a[s1] - a[s0])
        except IndexError:
            delta = 1.0
        return delta

class FFT1D(FFT):
    def __init__(self, x, s, detrend=True, window=False, lmap=True, **kwargs):
        FFT.__init__(self)
        # r-space
        self.x  = np.asanyarray(x)
        self.s  = np.asanyarray(s)

        assert len(self.x.shape) == 1
        assert self.x.shape == self.s.shape
        assert self.x.size == self.s.size

        # r-space spacing
        self.dx = self._delta(self.x, np.index_exp[0], np.index_exp[1])

        # r-space samples
        self.n0 = self.x.shape[0]

        # r-space lengths
        self.lx = self.n0 * self.dx

        # k-space
        self.u  = fftpack.fftshift(fftpack.fftfreq(self.n0))

        # k-space spacing
        self.du = self._delta(self.x, np.index_exp[0], np.index_exp[1])

        # k-space lengths
        self.lu = self.n0 * self.du

        # nyquist
        try:
            self.nyquist_u = 0.5/self.dx
        except ZeroDivisionError:
            self.nyquist_u = 0.0

        self.k = self.u

        if detrend:
            self.s = signal.detrend(self.s)

        if window:
            self._window()
            self.s = self.s * self.window

        self.fft = fftpack.fftshift(fftpack.fft(self.s))
        self.power = self.fft.real**2 + self.fft.imag**2

        if lmap:
            self.power = lm.surface.linear_mapping(self.power)

    def _window(self):
        self.window = signal.hamming(self.u.size)

    def __str__(self):
        s  = '[FFT1D]\n'
        s += '  %-6s = %s, %s\n' % ('shape', (self.n0))
        return s

class FFT2D(FFT):
    def __init__(self, x, y, s, detrend=True, window=False, lmap=True, **kwargs):
        FFT.__init__(self)
        # r-space
        self.x  = np.asanyarray(x)
        self.y  = np.asanyarray(y)
        self.s  = np.asanyarray(s)

        assert len(self.x.shape) == 2
        assert self.x.shape == self.y.shape == self.s.shape
        assert self.x.size == self.y.size == self.s.size

        # r-space spacing
        self.dx = self._delta(self.x, np.index_exp[0,0], np.index_exp[1,0])
        self.dy = self._delta(self.y, np.index_exp[0,0], np.index_exp[0,1])

        # r-space samples
        self.n0 = self.x.shape[0]
        self.n1 = self.x.shape[1]

        # r-space lengths
        self.lx = self.n0 * self.dx
        self.ly = self.n1 * self.dy

        # k-space
        u = fftpack.fftshift(fftpack.fftfreq(self.n0))
        v = fftpack.fftshift(fftpack.fftfreq(self.n1))
        self.u, self.v = np.meshgrid(u, v, indexing='ij')

        # k-space spacing
        self.du = self._delta(self.u, np.index_exp[0,0], np.index_exp[1,0])
        self.dv = self._delta(self.v, np.index_exp[0,0], np.index_exp[0,1])

        # k-space lengths
        self.lu = self.n0 * self.du
        self.lv = self.n1 * self.dv

        # nyquist
        try:
            self.nyquist_u = 0.5/self.dx
        except ZeroDivisionError:
            self.nyquist_u = 0.0

        try:
            self.nyquist_v = 0.5/self.dy
        except ZeroDivisionError:
            self.nyquist_v = 0.0

        self.k = np.sqrt(self.u**2 + self.v**2)

        if detrend:
            self.s = signal.detrend(self.s)

        if window:
            self._window()
            self.s = self.s * self.window

        self.fft = fftpack.fftshift(fftpack.fft2(self.s))
        self.power = self.fft.real**2 + self.fft.imag**2

        if lmap:
            self.power = lm.surface.linear_mapping(self.power)

    def _window(self):
            win_u  = signal.hamming(self.u.size)
            win_v  = signal.hamming(self.v.size)
            self.window = np.outer(win_u, win_v)
            self.window = np.reshape(self.window, self.s.shape)

    def __str__(self):
        s  = '[FFT2D]\n'
        s += '  %-6s = %s, %s\n' % ('shape', (self.n0, self.n1))
        return s

class FFT3D(FFT):
    def __init__(self, x, y, z, s, detrend=True, window=False, lmap=True, **kwargs):
        FFT.__init__(self)
        # r-space
        self.x  = np.asanyarray(x)
        self.y  = np.asanyarray(y)
        self.z  = np.asanyarray(z)
        self.s  = np.asanyarray(s)

        assert len(self.x.shape) == 3
        assert self.x.shape == self.y.shape == self.z.shape == self.s.shape
        assert self.x.size == self.y.size == self.z.size == self.s.size

        # r-space spacing
        self.dx = self._delta(self.x, np.index_exp[0,0,0], np.index_exp[1,0,0])
        self.dy = self._delta(self.y, np.index_exp[0,0,0], np.index_exp[0,1,0])
        self.dz = self._delta(self.z, np.index_exp[0,0,0], np.index_exp[0,0,1])

        # r-space samples
        self.n0 = self.x.shape[0]
        self.n1 = self.x.shape[1]
        self.n2 = self.x.shape[2]

        # r-space lengths
        self.lx = self.n0 * self.dx
        self.ly = self.n1 * self.dy
        self.lz = self.n2 * self.dz

        # k-space
        u = fftpack.fftshift(fftpack.fftfreq(self.n0))
        v = fftpack.fftshift(fftpack.fftfreq(self.n1))
        w = fftpack.fftshift(fftpack.fftfreq(self.n2))
        self.u, self.v, self.w = np.meshgrid(u, v, w, indexing='ij')

        # k-space samples
        self.du = self._delta(self.u, np.index_exp[0,0,0], np.index_exp[1,0,0])
        self.dv = self._delta(self.v, np.index_exp[0,0,0], np.index_exp[0,1,0])
        self.dw = self._delta(self.w, np.index_exp[0,0,0], np.index_exp[0,0,1])

        # k-space lengths
        self.lu = self.n0 * self.du
        self.lv = self.n1 * self.dv
        self.lw = self.n2 * self.dw

        # nyquist
        try:
            self.nyquist_u = 0.5/self.dx
        except ZeroDivisionError:
            self.nyquist_u = 0.0

        try:
            self.nyquist_v = 0.5/self.dy
        except ZeroDivisionError:
            self.nyquist_v = 0.0

        try:
            self.nyquist_w = 0.5/self.dz
        except ZeroDivisionError:
            self.nyquist_w = 0.0

        self.k = np.sqrt(self.u**2 + self.v**2 + self.w**2)

        if detrend:
            self.s = signal.detrend(self.s)

        if window:
            self._window()
            self.s = self.s * self.window

        self.fft = fftpack.fftshift(fftpack.fftn(self.s))
        self.power = self.fft.real**2 + self.fft.imag**2

        if lmap:
            self.power = lm.surface.linear_mapping(self.power)

    def _window(self):
        win_u  = signal.hamming(self.u.size)
        win_v  = signal.hamming(self.v.size)
        win_w  = signal.hamming(self.w.size)
        self.window = np.outer(np.outer(win_u, win_v), win_w)
        self.window = np.reshape(self.window, self.s.shape)

    def __str__(self):
        s  = '[FFT3D]\n'
        s += '  %-6s = %s, %s\n' % ('shape', (self.n0, self.n1, self.n2))
        return s