# -*- coding: utf-8 -*-
"""
Created on Tue Jun 26 09:29:56 2012

@author: adam
"""

import matplotlib.pyplot as plt
import numpy as np
import matplotlib
import shutil
import os

import langmuir
import adam

class Grid:
    """A grid to draw trap sites on.

    The grid contains all zeros, indicating that no traps are placed in the
    system.  Use the methods (ex: plane_xy) to draw trap sites or change
    grid.data directly to draw sites.  There are also methods to view the
    grid or save a simulation input file.
    """

    def __init__(self, xsize, ysize, zsize):
        self.xsize  = xsize
        self.ysize  = ysize
        self.zsize  = zsize
        self.xyArea = xsize * ysize
        self.xzArea = xsize * zsize
        self.yzArea = ysize * zsize
        self.volume = xsize * ysize * zsize
        self.data   = np.zeros((xsize, ysize, zsize), dtype=np.float64)

        rgbi  = (1, 0, 0)
        rgbf  = (0, 1, 0)
        cdict = dict(red   = [(0, 0, rgbi[0]), (1, rgbf[0], 0)],
                     green = [(0, 0, rgbi[1]), (1, rgbf[1], 0)],
                     blue  = [(0, 0, rgbi[2]), (1, rgbf[2], 0)])
        self.cm = matplotlib.colors.LinearSegmentedColormap('cm', cdict)

    def cube(self, x, dx, y, dy, z, dz, value=0.1):
        """Draw a cube of traps

        The cube as a (length, width, height) of (dx, dy, dz) and
        lower back left cornor of (x, y, z)

        Args:
            x     : the lower front left cornor x-value
            dx    : the width of the plane
            y     : the lower front left cornor y-value
            dy    : the height of the plane
            z     : the lower front left cornor z-value
            dz    : the height of the plane
            value : trap energy value in eV
        """
        self.data[x:x+dx,y:y+dy,z:z+dz] = value

    def square_xy(self, x, dx, y, dy, z=0, value=0.1):
        """Draw a square of traps in the xy plane

        The plane is parallel to the xy-plane, has a thickness 1,
        cornor at (x, y), and (width,height) of (dx, dy).

        Args:
            x     : the lower left cornor x-value
            dx    : the width of the plane
            y     : the lower left cornor y-value
            dy    : the height of the plane
            value : trap energy value in eV
        """
        self.data[x:x+dx,y:y+dy,z] = value

    def square_xz(self, x, dx, z, dz, y=0, value=0.1):
        """Draw a square of traps in the xy plane

        The plane is parallel to the xz-plane, has a thickness 1,
        cornor at (x, z), and (width,height) of (dx, dz).

        Args:
            x     : the lower left cornor x-value
            dx    : the width of the plane
            z     : the lower left cornor z-value
            dz    : the height of the plane
            value : trap energy value in eV
        """
        self.data[x:x+dx,y,z:z+dz] = value

    def square_yz(self, y, dy, z, dz, x=0, value=0.1):
        """Draw a square of traps in the xy plane

        The plane is parallel to the yz-plane, has a thickness 1,
        cornor at (y, z), and (width,height) of (dy, dz).

        Args:
            y     : the lower left cornor y-value
            dy    : the height of the plane
            z     : the lower left cornor z-value
            dz    : the width of the plane
            value : trap energy value in eV
        """
        self.data[x,y:y+dy,z:z+dz] = value

    def plane_xy(self, z, dz, value=1.0):
        """Draw an xy plane of traps

        The plane is parallel to the xy-plane and has a thickness dz
        and is located at z.

        Args:
            z     : distance of plane from the zero xy-plane
            dz    : thickness of plane
            value : trap energy value in eV
        """
        self.data[:,:,z:z+dz] = value

    def plane_yz(self, x, dx, value=1.0):
        """Draw an yz plane of traps

        The plane is parallel to the yz-plane and has a thickness dx
        and is located at x.

        Args:
            x     : distance of plane from the zero yz-plane
            dx    : thickness of plane
            value : trap energy value in eV
        """
        self.data[x:x+dx,:,:] = value

    def plane_xz(self, y, dy, value=1.0):
        """Draw an xz plane of traps

        The plane is parallel to the xz-plane and has a thickness dy
        and is located at y.

        Args:
            y     : distance of plane from the zero xz-plane
            dy    : thickness of plane
            value : trap energy value in eV
        """
        self.data[y:y+dy,:,:] = value

    def stripe_dx(self, dx, value=1.0):
        """Stack slabs along the x-direction of thickness dx.

        Draw alternating rectangular prisms with of thickness dx forall x, y

        Args:
            dx    : thickness of slab
            value : trap energy value in eV
        """
        for xi in range(0, self.ysize, 2*dx):
            self.data[xi:xi+dx,:,:] = value

    def stripe_dy(self, dy, value=1.0):
        """Stack slabs along the y-direction of thickness dy.

        Draw alternating rectangular prisms with of thickness dy forall x, z

        Args:
            dy    : thickness of slab
            value : trap energy value in eV
        """
        for yi in range(0, self.ysize, 2*dy):
            self.data[:,yi:yi+dy,:] = value

    def stripe_dz(self, dz, value=1.0):
        """Stack slabs along the z-direction of thickness dz.

        Draw alternating rectangular prisms with of thickness dz forall x, y

        Args:
            dz    : thickness of slab
            value : trap energy value in eV
        """
        for zi in range(0, self.ysize, 2*dz):
            self.data[:,:,zi:zi+dz] = value

    def checkerboard_xy(self, dx, dy, value=1.0):
        """Extend a checkerboard pattern along the z-direction.

        Draw a checkerboard pattern in xy plane, for all z values

        Args:
            dx    : x-size of checkerboard square
            dy    : y-size of checkerboard square
            value : trap energy value in eV
        """
        for xi in range(0, self.xsize, 2*dx):
            for yi in range(0, self.ysize, 2*dy):
                self.data[xi:xi+dx,yi:yi+dy,:] = value
        for xi in range(dx, self.xsize, 2*dx):
            for yi in range(dy, self.ysize, 2*dy):
                self.data[xi:xi+dx,yi:yi+dy,:] = value

    def checkerboard_yz(self, dy, dz, value=1.0):
        """Extend a checkerboard pattern along the x-direction.

        Draw a checkerboard pattern in yz plane, for all x values

        Args:
            dy    : y-size of checkerboard square
            dz    : z-size of checkerboard square
            value : trap energy value in eV
        """
        for yi in range(0, self.ysize, 2*dy):
            for zi in range(0, self.zsize, 2*dz):
                self.data[:,yi:yi+dy,zi:zi+dz] = value
        for yi in range(dy, self.ysize, 2*dy):
            for zi in range(dz, self.zsize, 2*dz):
                self.data[:,yi:yi+dy,zi:zi+dz] = value

    def showxy(self, z=0, name=None):
        """Matplotlib imshow of the xy-plane

        Args:
            z    : the z-value of the xy-plane to show
            name : if not None, save the image to a file
        """
        fig, ax = plt.subplots(1, 1)
        view = np.fliplr(np.rot90(self.data[:,:,z]))
        plt.imshow(view, cmap=self.cm, extent=[0, self.xsize, 0, self.ysize])
        ax1 = plt.gca()
        plt.ylim(0, self.ysize)
        plt.xlim(0, self.xsize)
        plt.title('z = %d' % z)
        plt.xlabel('x')
        plt.ylabel('y')
        ax1.xaxis.set_major_locator(matplotlib.ticker.MultipleLocator(32))
        ax1.yaxis.set_major_locator(matplotlib.ticker.MultipleLocator(32))
        b = ax1.get_position()
        c = fig.add_axes([b.x0 + b.width + 0.025, b.y0, 0.025, b.height])
        plt.colorbar(cax=c, ticks = matplotlib.ticker.MaxNLocator(1))
        for tick in ax1.xaxis.get_major_ticks():
            tick.tick1On = False
            tick.tick2On = False
        for tick in ax1.yaxis.get_major_ticks():
            tick.tick1On = False
            tick.tick2On = False
        if not name is None:
            adam.plot.save(name)
        else:
            plt.show()

    def showyz(self, x=0, name=None):
        """Matplotlib imshow of the yz-plane

        Args:
            x    : the x-value of the yz-plane to show
            name : if not None, save the image to a file
        """
        fig, ax = plt.subplots(1, 1)
        view = np.fliplr(np.rot90(self.data[x,:,:]))
        plt.imshow(view, cmap=self.cm, extent=[0, self.ysize, 0, self.zsize])
        ax1 = plt.gca()
        plt.xlim(0, self.ysize)
        plt.ylim(0, self.zsize)
        plt.title('x = %d' % x)
        plt.xlabel('y')
        plt.ylabel('z')
        ax1.xaxis.set_major_locator(matplotlib.ticker.MultipleLocator(32))
        ax1.yaxis.set_major_locator(matplotlib.ticker.MultipleLocator(16))
        for tick in ax1.xaxis.get_major_ticks():
            tick.tick1On = False
            tick.tick2On = False
        for tick in ax1.yaxis.get_major_ticks():
            tick.tick1On = False
            tick.tick2On = False
        if not name is None:
            adam.plot.save(name)
        else:
            plt.show()

    def showxz(self, y=0, name=None):
        """Matplotlib imshow of the xz-plane

        Args:
            y    : the z-value of the xz-plane to show
            name : if not None, save the image to a file
        """
        fig, ax = plt.subplots(1, 1)
        view = np.fliplr(np.rot90(self.data[:,y,:]))
        plt.imshow(view, cmap=self.cm, extent=[0, self.xsize, 0, self.zsize])
        ax1 = plt.gca()
        plt.xlim(0, self.xsize)
        plt.ylim(0, self.zsize)
        plt.title('y = %d' % y)
        plt.xlabel('x')
        plt.ylabel('z')
        ax1.xaxis.set_major_locator(matplotlib.ticker.MultipleLocator(32))
        ax1.yaxis.set_major_locator(matplotlib.ticker.MultipleLocator(16))
        for tick in ax1.xaxis.get_major_ticks():
            tick.tick1On = False
            tick.tick2On = False
        for tick in ax1.yaxis.get_major_ticks():
            tick.tick1On = False
            tick.tick2On = False
        if not name is None:
            adam.plot.save(name)
        else:
            plt.show()

    def contour3D(self, name=None, contours=3):
        """A 3D contour plot of the traps

        Args:
            name : if not None, save the image to a file
        """
        from mayavi import mlab
        if not name is None:
            mlab.options.offscreen = True
        mlab.figure(bgcolor=(1,1,1), fgcolor=(0,0,0))
        mlab.contour3d(self.data, transparent=True, contours=3)
        if not name is None:
            mlab.savefig(name, size=(1024, 1024))
            adam.plot.crop(name)
        else:
            mlab.axes(extent=[0, self.xsize, 0, self.ysize, 0, self.zsize],
                  xlabel='x', ylabel='y', zlabel='z')
            mlab.show()

    def save(self, name, template=None):
        """Save the information to a langmuir input file

        Args:
            name     : name of checkpoint file
            template : a template checkpoint file to inherit parameters from
        """
        if template is None:
            chk = langmuir.checkpoint.CheckPoint()
        else:
            chk = langmuir.checkpoint.CheckPoint(template)

        imap = langmuir.utils.IndexMapper(self.xsize, self.ysize, self.zsize)

        trapSites, trapValues = zip( *( (imap.s(x, y, z), v) for
            (x, y, z), v in np.ndenumerate(self.data) if not v == 0.0) )
        trapPercent = float(len(trapSites)+0.5)/float(self.volume)

        chk.fluxState      = []
        chk.randomState    = []
        chk.electrons      = []
        chk.holes          = []
        chk.traps          = trapSites
        chk.trapPotentials = trapValues

        chk.parameters['random.seed'    ] = 0
        chk.parameters['grid.x'         ] = self.xsize
        chk.parameters['grid.y'         ] = self.ysize
        chk.parameters['grid.z'         ] = self.zsize
        chk.parameters['trap.percentage'] = trapPercent

        chk.save(name)

        print chk

        path = os.path.dirname(name)

        #self.showxy(name=os.path.join(path, 'xy.png'))
        #self.showxz(name=os.path.join(path, 'xz.png'))
        #self.showyz(name=os.path.join(path, 'yz.png'))
        self.contour3D(name=os.path.join(path, 'band3D.png'))

if __name__ == '__main__':

#bar 2D
#    xsize=256
#    ysize=256
#    zsize=1
#    stub='/home/adam/Desktop/bar_256x256_re0+0_right%d/'
#    base='/home/adam/Desktop/'
#    handle = open('/home/adam/Desktop/submit', 'w')
#    print >> handle, '#!/bin/bash\n'
#    for width in [128]:
#        print >> handle, 'cd %s' % os.path.relpath(stub % width, base)
#        print >> handle, 'qsub -N w%d run.batch' % width
#        print >> handle, 'cd %s\n' % os.path.relpath(base, stub % width)
#        grid = Grid(xsize, ysize, zsize)
#        grid.square_xy(x=0, dx=width, y=0, dy=ysize, z=0, value=0.5)
#        os.makedirs(stub % width)
#        grid.save(os.path.join(stub % width, 'sim.inp'), 'template.inp')
#        shutil.copy('run.batch', stub % width)
#    handle.close()
#    os.system('chmod +x /home/adam/Desktop/submit')

 #comb3D
#    xsize = [128]
#    ysize = [128]
#    zsize = [8, 16, 32]
#    dy    = [4, 8, 16, 32]
#    dcomb = [8]
#
#    combinations = itertools.product(xsize, ysize, zsize, dy, dcomb)
#    for i, (xsize, ysize, zsize, dy, dcomb) in enumerate(combinations):
#        slab = 'band.128/grid.z_%d/slab.dy_%d/' % (zsize, dy)
#        print os.path.exists(slab)
#        os.makedirs(slab)
#        grid = Grid(xsize, ysize, zsize)
#        grid.stripe_dy(dy, value=0.5)
#        grid.plane_yz(0, dcomb, value=0)
#        grid.plane_yz(xsize-dcomb, dcomb, value=0.5)
#        grid.save(os.path.join(slab, 'sim.inp'), 'template.inp')
#
#        if not zsize == dy:
#            spag = 'band.128/grid.z_%d/spaghetti.dyz_%d' % (zsize, dy)
#            os.makedirs(spag)
#            grid = Grid(xsize, ysize, zsize)
#            grid.checkerboard_yz(dy, dy, value=0.5)
#            grid.plane_yz(0, dcomb, value=0)
#            grid.plane_yz(xsize-dcomb, dcomb, value=0.5)
#            grid.save(os.path.join(spag, 'sim.inp'), 'template.inp')
#    for p2 in range(2, 4):
#
#        dy = 2 ** p2
#
#        path = 'band.128/grid.z_%d/slab.dy_%d' % (zsize, dy)
#        os.makedirs(path)
#        grid = Grid(xsize, ysize, zsize)
#        grid.stripe_dy(dy, value=0.5)
#        grid.save(os.path.join(path, 'sim.inp'), 'template.inp')
#
#        if not zsize == dy:
#
#            path = 'band.128/grid.z_%d/spaghetti.dyz_%d' % (zsize, dy)
#            os.makedirs(path)
#            grid = Grid(xsize, ysize, zsize)
#            grid.checkerboard_yz(dy, dy, value=0.5)
#            grid.save(os.path.join(path, 'sim.inp'), 'template.inp')

    grid = Grid(196, 196, 24)
    grid.checkerboard_yz(8, 8, value=-0.5)
    #grid.contour3D()
    grid.save('sim.inp', 'template.inp')

# band3D
#    xsize = 128
#    ysize = 128
#    zsize = 8
#    dy    = 16
#
#    for p2 in range(2, 4):
#
#        dy = 2 ** p2
#
#        path = 'band.128/grid.z_%d/slab.dy_%d' % (zsize, dy)
#        os.makedirs(path)
#        grid = Grid(xsize, ysize, zsize)
#        grid.stripe_dy(dy, value=0.5)
#        grid.save(os.path.join(path, 'sim.inp'), 'template.inp')
#
#        if not zsize == dy:
#
#            path = 'band.128/grid.z_%d/spaghetti.dyz_%d' % (zsize, dy)
#            os.makedirs(path)
#            grid = Grid(xsize, ysize, zsize)
#            grid.checkerboard_yz(dy, dy, value=0.5)
#            grid.save(os.path.join(path, 'sim.inp'), 'template.inp')

#band2D
#    work = os.getcwd()
#    template = 'input/template.inp'
#    for power in range(2, 7):
#        size = 2 ** power
#        grid = Grid(256, 256, 1)
#        grid.stripe_dy(size, value=0.5)
#        path = 'grid.z_%d' % size
#        os.makedirs(path)
#        grid.save(os.path.join(path, 'sim.inp'), template)
