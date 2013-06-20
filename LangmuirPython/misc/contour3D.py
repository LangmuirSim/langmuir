# -*- coding: utf-8 -*-
"""
Created on Thu Jan 10 11:05:09 2013

@author: adam
"""

from scipy.interpolate import griddata
import numpy as np
import argparse
import sys
import os

def get_arguments(args=None):
    """
    Get command line arguments

    args: list of strings that will override the command line parameters
    """
    parser = argparse.ArgumentParser()
    parser.description = 'plot contour isosurface of x, y, z, v data in text file'
    parser.add_argument(dest='input', help='input file name', metavar='input',
                        default='plot.esp', nargs='?')
    parser.add_argument(dest='output', help='output file name', metavar='output',
                        default=None, nargs='?')
    parser.add_argument('--show', action='store_true', default=False,
                        help='show plot in mayavi window')
    parser.add_argument('--mesh', nargs=3, type=int, default=[10, 10, 10],
                        help='mesh points along each direction', metavar='int')
    opts = parser.parse_args(args)
    parser.print_help()
    print ''
    print 'parameters:'
    for key, value in opts.__dict__.iteritems():
        print '\t%-10s = %s' % (key, value)
    print ''

    if not os.path.exists(opts.input):
        print >> sys.stderr, 'error: input file %s does not exist' % opts.input
        sys.exit(-1)

    if opts.output is None and not opts.show:
        print >> sys.stderr, 'error: nothing to do, use --show or provide an ' + \
        'output file name'
        sys.exit(-1)

    return parser, opts

def create_example_data():
    """
    Generates sample data and saves it in sample.dat
    """
    x, y, z = np.mgrid[0:10:25j, 0:10:25j, 0:10:25j]
    v = np.cos(x) * np.sin(y) - np.cos(y) * np.sin(x) + z

    data = np.zeros(shape=(v.size, 4))
    for i, ((xi, yi, zi), v) in enumerate(np.ndenumerate(v)):
        data[i] = x[xi, yi, zi], y[xi, yi, zi], z[xi, yi, zi], v

    np.savetxt('sample.dat', data)

def load_data(fname, nx=10, ny=10, nz=10, xcol=0, ycol=1, zcol=2, vcol=3,
              method='nearest'):
    """
    Load x,y,z,v data from a text file and put it on a mesh

    fname  : name of text file
    nx     : number of mesh points along x direction
    ny     : number of mesh points along y direction
    nz     : number of mesh points along z direction
    xcol   : column index of x data in text file
    ycol   : column index of y data in text file
    zcol   : column index of z data in text file
    vcol   : column index of v data in text file
    method : interpolation method used (see scipy.interpolate.griddata)
    """

    print 'loading data from %s' % fname
    data = np.loadtxt(fname)

    x = data[:,xcol]
    y = data[:,ycol]
    z = data[:,zcol]
    v = data[:,vcol]

    xmin, xmax = np.amin(x), np.amax(x)
    ymin, ymax = np.amin(y), np.amax(y)
    zmin, zmax = np.amin(z), np.amax(z)

    print 'creating mesh xdim=(%g:%g:%g) ydim=(%g:%g:%g) zdim=(%g:%g:%g)' % (
              xmin, xmax, nx, ymin, ymax, ny, zmin, zmax, nz)

    mesh_x, mesh_y, mesh_z = np.mgrid[xmin:xmax:complex(nx),
                                      ymin:ymax:complex(ny),
                                      zmin:zmax:complex(nz)]

    print 'performing interpolation of data to mesh points (slow)'
    mesh_v = griddata((x, y, z), v, (mesh_x, mesh_y, mesh_z), method='linear')
    print 'interpolation finished'

    return mesh_x, mesh_y, mesh_z, mesh_v

def plot(x, y, z, v, bgcolor=(1,1,1), fgcolor=(0,0,0), figx=512, figy=512,
         contours=10, vmin=None, vmax=None, opacity=0.5, transparent=False,
         colormap='Spectral', outline=True, axes=True, colorbar=True):
    """
    Create contour isosurface of x, y, z, v data

    x           : x values - np array of shape (nx, ny, nz)
    y           : y values - np array of shape (nx, ny, nz)
    z           : z values - np array of shape (nx, ny, nz)
    v           : v values - np array of shape (nx, ny, nz)
    fgcolor     : foreground color rgb-tuple
    bgcolor     : background color rgb-tuple
    figx        : figure size, x-direction
    figy        : figure size, y-direction
    contours    : number of contours, or a list of contour levels
    vmin        : minimum value of colorbar
    vmax        : maximum value of colorbar
    opacity     : transparency of contour surfaces
    transparent : make opacity depend upon v
    colormap    : name of colormap (see mayavi doc for possibilities)
    outline     : draw a box around contours
    axes        : draw axes
    colorbar    : draw colorbar
    """

    print 'creating figure'
    figure = mlab.figure(bgcolor=bgcolor, fgcolor=fgcolor, size=(figx, figy))
    figure.scene.disable_render = True

    print 'plotting contours'
    mlab.contour3d(x, y, z, v, contours=contours, vmin=vmin, vmax=vmax,
                   opacity=opacity, transparent=transparent, colormap=colormap)
    if outline:
        print 'creating outline'
        mlab.outline()

    if colorbar:
        print 'creating colorbar'
        mlab.colorbar(nb_labels=6)

    if axes:
        print 'creating axes'
        mlab.axes(line_width=3, nb_labels=5, xlabel='x', ylabel='y', zlabel='z')

    print 'adjusting eyes'
    azimuth, zenith, distance, focalpoint = mlab.view()
    mlab.view(distance=1.5*distance)

    return figure

if __name__ == '__main__':

    parser, opts = get_arguments()

    print 'importing mayavi (slow)'
    import mayavi.mlab as mlab

    nx, ny, nz = opts.mesh
    x, y, z, v = load_data(opts.input, nx=opts.mesh[0], ny=opts.mesh[1],
                           nz=opts.mesh[2])
    figure = plot(x, y, z, v)

    if not opts.output is None:
        figure.scene.disable_render = False
        mlab.save(opts.output)

    if opts.show:
        figure.scene.disable_render = False
        mlab.show()
