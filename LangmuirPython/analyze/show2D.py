# -*- coding: utf-8 -*-
"""
Created on Thu Jul  5 08:31:30 2012

@author: adam
"""

import matplotlib.pyplot as plt
import matplotlib as mpl
import langmuir
import argparse
import os

_width = 2
mpl.rcParams['axes.linewidth'] = _width

ecolor = 'pink'
eline  = 'r'
hcolor = 'cyan'
hline  = 'b'
ocolor = None
pcolor = 'r'
ncolor = 'k'

def get_arguments(args=None):
    parser = argparse.ArgumentParser(
        description="""
        render traps, electrons, holes, and defects in 2D using information
        in a langmuir checkpoint file.
        """)
    parser.add_argument(dest='input', type=str, default='out.chk.gz', nargs='?',
                        help='name of input file', metavar='input.chk')
    parser.add_argument(dest='output', type=str, default=None, nargs='?',
                        help='name of output file', metavar='output.png')
    parser.add_argument('--show', action='store_true',  default=False,
                        help='show the result in a Matplotlib window')
    parser.add_argument('--elecs', action='store_true', default=False,
                        help='render electrons')
    parser.add_argument('--holes', action='store_true', default=False,
                        help='render holes')
    parser.add_argument('--zlevel', default=0, type=int,
                        help='which layer to draw')
    opts = parser.parse_args(args)

    return parser, opts

def get_cmap():
    cmap = mpl.colors.ListedColormap(colors=(ncolor, pcolor))
    cmap._init()
    alphas = [0.4, 0.6]
    cmap._lut[:-3,-1] = alphas
    return cmap

def plot2D():

    chk  = langmuir.checkpoint.load(opts.input)
    grid = langmuir.grid.grid_from_checkpoint(chk)
    cmap = get_cmap()

    if chk.trapPotentials:
        trap_xyzv = langmuir.grid.XYZV(grid, chk.traps, chk.trapPotentials)

    elif chk.parameters.has_key('trap.potential'):
        trap_xyzv = langmuir.grid.XYZV(grid, chk.traps,
                                       chk['trap.potential'])

    else:
        trap_xyzv = langmuir.grid.XYZV(grid, chk.traps)

    elec_xyzv = langmuir.grid.XYZV(grid, chk.electrons, -1)
    hole_xyzv = langmuir.grid.XYZV(grid, chk.holes, +1)

    x = trap_xyzv.mesh_x[:,:,opts.zlevel]
    y = trap_xyzv.mesh_y[:,:,opts.zlevel]
    z = trap_xyzv.mesh_v[:,:,opts.zlevel]

    fig, ax1 = plt.subplots(1, 1, figsize=(10, 10))
    plt.subplots_adjust(left=0.2, right=0.8, bottom=0.2, top=0.8)

    plt.contourf(x, y, z, v=3, cmap=cmap, zorder=-2)

    if not ocolor is None:
        plt.contour(x, y, z, levels=[0, -0.5], colors=ocolor,
                    linestyles='solid', linewidths=3, zorder=-1)

    if opts.elecs:
        try:
            x, y = zip(*[(x, y) for x, y, z in zip(elec_xyzv.site_x,
                         elec_xyzv.site_y, elec_xyzv.site_z)
                             if z == opts.zlevel])
            plt.plot(x, y, marker='o', mfc=ecolor, mec=eline, ls='none',
                     ms=4, alpha=1)
        except:
            print 'warning : no electrons to plot'

    if opts.holes:
        try:
            x, y = zip(*[(x, y) for x, y, z in zip(hole_xyzv.site_x,
                hole_xyzv.site_y, hole_xyzv.site_z) if z == opts.zlevel])
            plt.plot(x, y, marker='o', mfc=hcolor, mec=hline, ls='none',
                     ms=4, alpha=1)
        except:
            print 'warning : no holes to plot'

    plt.gca().xaxis.set_major_locator(mpl.ticker.MultipleLocator(64))
    plt.gca().xaxis.set_minor_locator(mpl.ticker.MultipleLocator(32))
    plt.gca().yaxis.set_major_locator(mpl.ticker.MultipleLocator(64))
    plt.gca().yaxis.set_minor_locator(mpl.ticker.MultipleLocator(32))

    plt.xlim(0, grid.xlength)
    plt.ylim(0, grid.ylength)

    plt.xlabel('x (nm)')
    plt.ylabel('y (nm)')

    plt.tick_params(axis='both', which='both', direction='out')

    if opts.output:
        langmuir.plot.save(opts.output)

    if opts.show:
        plt.show()

if __name__ == '__main__':
    parser, opts = get_arguments()
    work = os.getcwd()
    plot2D()