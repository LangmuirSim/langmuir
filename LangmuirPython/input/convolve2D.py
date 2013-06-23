# -*- coding: utf-8 -*-
"""
Created on Wed Nov 14 09:37:49 2012

@author: adam
"""

import matplotlib.pyplot as plt
import scipy.signal
import numpy as np

def Example1D():
    xmin, xmax = -2*np.pi, 2*np.pi
    x = np.linspace(xmin, xmax, 100)
    y = np.sin(x)
    f = np.random.random(x.size) - 0.5
    c = np.convolve(y, f, mode='same')
    plt.plot(x, y, ms='none', ls='-', c='r')
    plt.plot(x, f, ms='none', ls='-', c='b')
    plt.plot(x, c, ms='none', ls='-', c='g')
    plt.xlim(xmin, xmax)
    plt.show()

def Example2D():
    xmin, xmax, dx = 0, 256, 256j
    ymin, ymax, dy = 0, 256, 256j
    x, y = np.mgrid[xmin:xmax:dx,ymin:ymax:dy]
    
    shape, size = x.shape, x.size
    
    z = np.random.random(shape)
    z = np.reshape(z, size)  
    
    sx, sy = 128, 128
    f = np.exp(-((x-128)**2)/(2*(sx**2))) * np.exp(-((y-128)**2)/(2*(sy**2)))
    f = np.reshape(f, size)

    c = 0.5*(z + f)
    #c = np.convolve(z, f, mode='same')
    
    z = np.reshape(z, shape)
    f = np.reshape(f, shape)
    c = np.reshape(c, shape)
    
    k = np.copy(c)
    m = np.median(k)
    k[k >= m] = 1
    k[k != 1] = 0

    plt.figure()
    plt.imshow(z, extent=[xmin, xmax, ymin, ymax])
    
    plt.figure()
    plt.imshow(f, extent=[xmin, xmax, ymin, ymax])
        
    plt.figure()
    plt.imshow(c, extent=[xmin, xmax, ymin, ymax])

    plt.figure()
    plt.imshow(k, extent=[xmin, xmax, ymin, ymax])
    
    plt.show()

def Kernal2D(sigma_x, sigma_y=None):
    if sigma_y is None: sigma_y = sigma_x
    x, y = np.mgrid[-sigma_x:sigma_x:100j, -sigma_y:sigma_y:100j]
    g = np.exp(-(x**2/float(sigma_x)+y**2/float(sigma_y)))
    return g / g.sum()

def Example2D():
    xmin, xmax, dx = -70, 70, 100j
    ymin, ymax, dy = -70, 70, 100j
    x, y = np.mgrid[xmin:xmax:dx,ymin:ymax:dy]
    shape, size = x.shape, x.size
    z = np.cos((x**2 + y**2)/(200*np.sqrt(2))) + np.random.normal(size=shape)
    k = Kernal2D(3)
    c = scipy.signal.convolve(z, k, mode='same')
    plt.imshow(c, extent=[xmin, xmax, ymin, ymax])
    plt.show()

if __name__ == '__main__':
    Example2D()