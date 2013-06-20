# -*- coding: utf-8 -*-
"""
Created on Tue Feb  5 10:43:41 2013

@author: adam
"""

import langmuir
import argparse
import sys
import os

def get_arguments(args=None):
    desc = "Render vtk files created using chk2vtk.py"
    parser = argparse.ArgumentParser(description=desc)
    parser.add_argument('--backend', default='mayavi', type=str,
               choices=['paraview', 'mayavi', 'envisage'],
               help='rendering library (paraview, mayavi, envisage)')
    parser.add_argument('--traps', default='traps.vti', type=str,
               help='trap vti file')
    parser.add_argument('--elecs', default='elecs.vtu', type=str,
               help='elec vtu file')
    parser.add_argument('--holes', default='holes.vtu', type=str,
               help='hole vtu file')
    parser.add_argument('--show', action='store_true', default=False,
               help='open window')
    parser.add_argument(dest='output', nargs='?', default=None,
               help='output file name')
    opts = parser.parse_args(args)

    if not os.path.exists(opts.traps):
        opts.traps = None

    if not os.path.exists(opts.elecs):
        opts.elecs = None

    if not os.path.exists(opts.holes):
        opts.holes = None

    if not opts.show and not opts.output:
        print >> sys.stderr, \
            'error: nothing to do (use --show or name the output file)'
        sys.exit(-1)

    return opts

def backend_mayavi():

    print 'importing mayavi.mlab (slow)'
    import mayavi.mlab as mlab

    if opts.backend == 'envisage':
        mlab.options.backend = 'envisage'

    if not opts.show:
        mlab.options.offscreen = True

    print 'creating figure'
    fig = mlab.figure(fgcolor=(0, 0, 0), bgcolor=(1, 1, 1), size=(1024, 1024))
    fig.scene.disable_render = True

    if opts.traps:
        print 'loading %s' % opts.traps
        source = mlab.pipeline.open(opts.traps)

        print 'creating isosurface'
        isosurface = mlab.pipeline.iso_surface(source, vmin=0.49, vmax=0.5,
            contours=3, colormap='autumn', name='trap isosurface')

        print 'creating outline'
        mlab.outline(isosurface)

        print 'creating axes'
        axes = mlab.axes(line_width=3, nb_labels=0, xlabel='X',
                         ylabel='Y', zlabel='Z')
        axes.axes.label_format = ''

    if opts.elecs:

        print 'loading %s' % opts.elecs
        source = mlab.pipeline.open(opts.elecs)

        print 'creating glyphs'
        glyph = mlab.pipeline.glyph(source, name='elec glyph', mode='sphere',
            scale_mode='none', color=(0, 0, 1), resolution=16)
        glyph.glyph.glyph_source.glyph_source.radius = 1

    if opts.holes:

        print 'loading %s' % opts.holes
        source = mlab.pipeline.open(opts.holes)

        print 'creating glyphs'
        glyph = mlab.pipeline.glyph(source, name='hole glyph', mode='sphere',
            scale_mode='none', color=(0, 0, 0), resolution=16)
        glyph.glyph.glyph_source.glyph_source.radius = 1

    print 'adjusting eyes'
    azimuth, zenith, distance, focalpoint = mlab.view()
    mlab.view(distance=1.10*distance)

    fig.scene.disable_render = False
    fig.scene.anti_aliasing_frames = 0

    if not opts.output is None:
        file_name = os.path.splitext(opts.output)[0] + '.png'
        print 'saving %s' % file_name
        mlab.savefig(os.path.splitext(opts.output)[0] + '.png')
        langmuir.plot.crop(file_name)

    if opts.show:
        print 'showing'
        mlab.show()

def backend_paraview():
    print 'paraview backend not implemented'

if __name__ == '__main__':

    work = os.getcwd()
    opts = get_arguments()

    if opts.backend is 'paraview':
        backend_paraview()
    else:
        backend_mayavi()
