# -*- coding: utf-8 -*-
"""
pepper.py
=========

.. argparse::
    :module: pepper
    :func: create_parser
    :prog: pepper

.. moduleauthor:: Adam Gagorik <adam.gagorik@gmail.com>
"""
import langmuir as lm
import numpy as np
import collections
import argparse
import sys
import os

desc = """
Add pepper to phase(s).
"""

brushes = {}
brushes['point' ] = np.array([[0, 0, 0]])
brushes['square'] = np.array([[0, 0, 0], [1, 0, 0], [0, 1, 0], [1, 1, 0]])
brushes['cross' ] = np.array([[ 0,  0,  0], [ 0,  0, -1], [0, 0, 1], [-1, 0, 0], [1, 0, 0], [0, -1, 0], [0, 1, 0]])

def create_parser():
    parser = argparse.ArgumentParser()
    parser.description = desc

    parser.add_argument(dest='ifile', type=str, metavar='input', help='input file')
    parser.add_argument(dest='ofile', type=str, metavar='output', help='output file')

    groupA = parser.add_mutually_exclusive_group(required=True)
    groupA.add_argument('--count', type=int, metavar='int', default=None, help='number of strokes')
    groupA.add_argument('--percent', type=float, metavar='float', default=None,
        help='calculate number of strokes based on percent of phase volume')

    parser.add_argument('--brush', type=str, metavar='str', default='point', choices=brushes.keys(),
        help='pepper brush type')
    parser.add_argument('--phase', type=int, metavar='int', default=0, help='phase ID')
    parser.add_argument('--overlap', action='store_true', help='allow overlap')
    parser.add_argument('--total', action='store_true', help='compute count based on total system')

    parser.add_argument('--show', action='store_true', help='show plot')

    return parser

def get_arguments(args=None):
    parser = create_parser()
    opts = parser.parse_args(args)
    return opts

def info(image):
    print '[Image]'
    print '  {0:10}: {1}, {2}, {3}'.format('shape', *image.shape)
    print '  {0:10}: {1}'.format('size', image.size)
    phases = np.unique(image)
    for i, phase in enumerate(phases):
        p = image[image==phases[i]]
        percent = p.size/float(image.size) * 100
        print '[Phase {0}]'.format(i)
        print '  {0:10}: {1}'.format('size', p.size)
        print '  {0:10}: {1:.3f}'.format('percent', percent)
    print ''

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()

    image = lm.surface.load(opts.ifile)

    phases = np.unique(image)

    if not phases.size == 2:
        raise RuntimeError('invalid phase count: %s' % phases.size)

    if not (0 <= opts.phase < phases.size):
        raise RuntimeError('invalid phase ID: %d' % opts.phase)

    info(image)

    x_i, y_i, z_i = np.where(image==phases[opts.phase])

    size = x_i.size
    if opts.count is None:
        if opts.total:
            opts.count = int(opts.percent * image.size)
        else:
            opts.count = int(opts.percent * size)

    print 'to_change: %d\n' % opts.count

    pixels_changed, tries = 0, 0
    while pixels_changed < opts.count:
        i = np.random.randint(0, x_i.size - 1)
        to_paint = brushes[opts.brush] + (x_i[i], y_i[i], z_i[i])

        x_j, y_j, z_j = to_paint[:,0], to_paint[:,1], to_paint[:,2]

        valid = ((x_j < image.shape[0]) & (x_j >= 0) &
                 (y_j < image.shape[0]) & (y_j >= 0) &
                 (z_j < image.shape[0]) & (z_j >= 0))

        x_j = x_j[valid]
        y_j = y_j[valid]
        z_j = z_j[valid]

        valid = (image[x_j, y_j, z_j] == phases[opts.phase])
        x_j = x_j[valid]
        y_j = y_j[valid]
        z_j = z_j[valid]

        if opts.overlap:
            image[x_j, y_j, z_j] = phases[opts.phase - 1]
            pixels_changed += x_j.size
        elif x_j.size == brushes[opts.brush].shape[0]:
            image[x_j, y_j, z_j] = phases[opts.phase - 1]
            pixels_changed += x_j.size

        #x_i, y_i, z_i = np.where(image==phases[opts.phase])
        tries += 1

        if tries > 1e9:
            raise RuntimeError, 'exceeded max tried: %d' % 1e9

    print '[Changed]'
    print '  {0:10}: {1}'.format('pixels', pixels_changed)
    print '  {0:10}: {1:.3f}'.format('percent', pixels_changed/float(size) * 100)
    print ''

    info(image)

    stub, ext = lm.common.splitext(opts.ofile)
    handle = lm.common.format_output(stub=stub, name='', ext=ext)
    print 'saved: %s' % handle
    if ext in ['.png', '.jpg', '.jpeg']:        
        lm.surface.save(handle, np.rot90(image, 1))
    else:
        lm.surface.save(handle, image)

    if opts.show:
        grid = lm.grid.Grid(*image.shape)
        if not image.shape.count(1):
            try:
                print 'importing mayavi...'
                import mayavi.mlab as mlab
                mlab.contour3d(grid.mx, grid.my, grid.mz, image, contours=3)
                mlab.show()
                sys.exit(-1)
            except ImportError:
                if not lm.vtk is None:
                    lm.vtkutils.show(lm.vtkutils.filter_marching(lm.vtkutils.create_image_data_from_array(image)))
                    sys.exit(-1)
                else:
                    print 'can not create 3D plot'

        lm.plot.contourf(grid.mx, grid.my, image, 3)
        lm.plot.plt.show()
