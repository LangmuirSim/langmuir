# -*- coding: utf-8 -*-
"""
Created on Wed Nov 14 13:03:51 2012

@author: adam
"""

from scipy.interpolate import griddata
from scipy.signal import convolve
import matplotlib.pyplot as plt
import numpy as np
import langmuir

def kernel(x, mu, var):
    return np.exp(-(x - mu)**2/(2 * var))

def kernel1D(varx, dx=1+1e-16):
    sx = 2 * varx
    x = np.mgrid[-sx:sx:dx]
    g = kernel(x, 0, varx)
    return g / g.sum()

def kernel2D(varx, vary, dx=1+1e-16, dy=1+1e-16):
    sx = 2 * varx
    sy = 2 * vary
    x, y = np.mgrid[-sx:sx:dx, -sy:sy:dy]
    g = kernel(x, 0, varx) * kernel(y, 0, vary)
    return g / g.sum()

def kernel3D(varx, vary, varz, dx=1+1e-16, dy=1+1e-16, dz=1+1e-16):
    sx = 2 * varx
    sy = 2 * vary
    sz = 2 * varz
    x, y, z = np.mgrid[-sx:sx:dx, -sy:sy:dy, -sz:sz:dz]
    g = kernel(x, 0, varx) * kernel(y, 0, vary) * kernel(z, 0, varz)
    return g / g.sum()

def noise(shape):
    return np.random.normal(size=shape) - 0.5

def threshold(data):
    array = np.copy(data)
    array = array - np.average(array)
    array[array >= 0] = 1
    array[array != 1] = 0
    return array

def demo1D():
    xmin, xmax, dx = 0, 256, 1
    varx = 18

    x = np.mgrid[xmin:xmax:dx]
    size, shape = x.size, x.shape
    g = kernel1D(varx)
    r = noise(shape)
    c = convolve(r, g, mode='same')
    #c = c - np.average(c)
    t = threshold(c)

    print 'x:', x.size, x.shape
    print 'g:', g.size, g.shape
    print 'r:', r.size, r.shape
    print 'c:', c.size, c.shape
    print 't:', t.size, t.shape

    plt.plot(g, 'r-')
    plt.plot(c, 'c-')
    plt.plot(t, 'g-')
    plt.xlim(xmin, xmax)
    plt.show()

def demo2D():
    xmin, xmax, dx = 0, 256, 1
    ymin, ymax, dy = 0, 256, 1
    varx, vary = 18, 18

    x, y = np.mgrid[xmin:xmax:dx, ymin:ymax:dy]
    size, shape = x.size, x.shape
    g = kernel2D(varx, vary)
    r = noise(shape)
    c = convolve(r, g, mode='same')
    c = c - np.average(c)
    t = threshold(c)

    print 'x:', x.size, x.shape
    print 'y:', y.size, y.shape
    print 'g:', g.size, g.shape
    print 'r:', r.size, r.shape
    print 'c:', c.size, c.shape
    print 't:', t.size, t.shape

    plt.imshow(g)
    plt.colorbar()
    plt.show()

def demo3D():
    xmin, xmax, dx = 0, 32, 0.5
    ymin, ymax, dy = 0, 32, 0.5
    zmin, zmax, dz = 0, 32, 0.5
    varx, vary, varz = 2, 2, 2

    print 'creating mesh...'
    x, y, z = np.mgrid[xmin:xmax:dx, ymin:ymax:dy, zmin:zmax:dz]
    size, shape = x.size, x.shape

    print 'creating kernel...'
    g = kernel3D(varx, vary, varz)

    print 'creating noise...'
    r = noise(shape)

    print 'convoluting...'
    c = convolve(r, g, mode='same')
    c = c - np.average(c)

    print 'thresholding...'
    t = threshold(c)

    print 'x:', x.size, x.shape
    print 'y:', y.size, y.shape
    print 'z:', y.size, y.shape
    print 'g:', g.size, g.shape
    print 'r:', r.size, r.shape
    print 'c:', c.size, c.shape
    print 't:', t.size, t.shape

    print 'saving...'
    extent = (0, shape[0] - 1, 0, shape[1] - 1, 0, shape[2] - 1)
    spacing = (dx, dy, dz)
    origin = (0, 0, 0)

    data_source = langmuir.vtkutil.create_vtkImageData(shape, extent, spacing,
                                                       origin)
    point_data = langmuir.vtkutil.create_vtkDoubleArray(c, 1)
    data_source.GetPointData().AddArray(point_data)
    langmuir.vtkutil.save_vtkImageData(data_source, '/home/adam/Desktop/hetero')

    print 'plotting...'
    #from mayavi import mlab
    #mlab.contour3d(g)
    #mlab.show()

if __name__ == '__main__':
    demo3D()

#def gaussian(x, mu, sigma):
#    return np.exp(-(x - mu)**2/(2 * sigma**2))
#
#def noise(shape):
#    return np.random.normal(size=shape)
#
#def kernel1D(x, mx, sx):
#    g = gaussian(x, mx, sx)
#    return g / g.sum()
#
#def kernel2D(x, y, mx, my, sx, sy):
#    g = gaussian(x, mx, sx) * gaussian(y, my, sy)
#    return g / g.sum()
#
#def kernel3D(x, y, z, mx, my, mz, sx, sy, sz):
#    g = gaussian(x, mx, sx) * gaussian(y, my, sy) * gaussian(z, mz, sz)
#    return g / g.sum()
#
#def threshold(im):
#    #avg = np.average(im)
#    res = np.copy(im)
#    res[res >=   0] = 1
#    res[res !=   1] = 0
#    return res
#
#def interpolateMesh2D(x, y, v, shape=(100, 100), method='linear'):
#    xmin, xmax = np.amin(x), np.amax(x)
#    ymin, ymax = np.amin(y), np.amax(y)
#    dx, dy = complex('%dj' % shape[0]), complex('%dj' % shape[1])
#    nx, ny = np.mgrid[xmin:xmax:dx, ymin:ymax:dy]
#    tx = np.reshape(x, x.size)
#    ty = np.reshape(y, y.size)
#    tv = np.reshape(v, v.size)
#    tv = griddata((tx, ty), tv, (nx, ny), method=method)
#    return (nx, ny), tv
#
#if __name__ == '__main__':
#
#    xmin, xmax, dx = 0, 100, 1
#    ymin, ymax, dy = 0, 100, 1
#    zmin, zmax, dz = 0, 100, 1
#
#    x, y = np.mgrid[xmin:xmax:dx, ymin:ymax:dy]
#    size, shape = x.size, x.shape
#
#    g = kernel2D(x, y, 50, 50, 0.5*np.sqrt(25), 0.5*np.sqrt(25)) + \
#        kernel2D(x, y, 25, 25, 0.5*np.sqrt(25), 0.5*np.sqrt(25)) + \
#        kernel2D(x, y, 75, 75, 0.5*np.sqrt(25), 0.5*np.sqrt(25)) + \
#        kernel2D(x, y, 75, 25, 0.5*np.sqrt(25), 0.5*np.sqrt(25)) + \
#        kernel2D(x, y, 25, 75, 0.5*np.sqrt(25), 0.5*np.sqrt(25))
#
#    r = noise(shape)
#    c = convolve(g, r, mode='same')
#    t = threshold(c)
#
#    #(nx, ny), v = interpolateMesh2D(x, y, t)
#
#    plt.imshow(t)
#    plt.show()
