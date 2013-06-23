# -*- coding: utf-8 -*-

import numpy as np
import langmuir
import os

class Surface:
    def __init__(self, px, py, pz, nx=1, ny=1, nz=1):
        self.grid = langmuir.grid.Grid()
        self.grid.init_vtk((0.5, 0.5, 0.5), (1, 1, 1), (px, py, pz))
        self.wave_x = langmuir.surface.WaveDimensions(self.grid.px, nx)
        self.wave_y = langmuir.surface.WaveDimensions(self.grid.py, ny)
        self.wave_z = langmuir.surface.WaveDimensions(self.grid.pz, nz)
        print self.grid
        print self.wave_x
        print self.wave_y
        print self.wave_z
        
        print 'creating mesh'
        self.mesh_x, self.mesh_y, self.mesh_z = self.grid.create_mgrid()

    def evaluate_with_wave_args(self, f, *args, **kwargs):
        print 'evaluating %s on mesh' % f.__name__
        self.mesh_v = f(self.mesh_x, self.mesh_y, self.mesh_z, self.wave_x.k,
                        self.wave_y.k, self.wave_z.k, *args, **kwargs)

    def evaluate(self, f, *args, **kwargs):
        print 'evaluating %s on mesh' % f.__name__
        self.mesh_v = f(self.mesh_x, self.mesh_y, self.mesh_z, *args)
    
    def threshold(self):
        print 'copying surface'
        self.mesh_t = np.copy(self.mesh_v)
        
        print 'performing threshold'
        self.mesh_t[self.mesh_t <= 0] = 0
        self.mesh_t[self.mesh_t >  0] = 0.5

        print 'calculating site ids'
        shape = (self.grid.px, self.grid.py, self.grid.pz)
        imap  = langmuir.grid.IndexMapper(shape)
        self.sites, self.values = zip(*((imap.indexS(x, y, z), v) for
            (x, y, z), v in np.ndenumerate(self.mesh_t) if not v == 0.0))        
    
    def save(self, name, template=None, parms=None):
        print 'creating %s' % os.path.dirname(name)
        if not os.path.exists(os.path.dirname(name)):
            os.makedirs(os.path.dirname(name))
    
        if template is None:
            chk = langmuir.checkpoint.CheckPoint()
            chk.parameters.set_defaults()
        else:
            chk = langmuir.checkpoint.CheckPoint(template)
    
        volume = self.grid.px * self.grid.py * self.grid.pz
        percent = float(len(self.sites)+0.5)/float(volume)
    
        chk.fluxState      = []
        chk.randomState    = []
        chk.electrons      = []
        chk.holes          = []
        chk.trapPotentials = []
        chk.traps          = self.sites
        chk.parameters['random.seed'    ] = 0
        chk.parameters['seed.charges'   ] = 0.0
        chk.parameters['grid.x'         ] = self.grid.px
        chk.parameters['grid.y'         ] = self.grid.py
        chk.parameters['grid.z'         ] = self.grid.pz
        chk.parameters['trap.percentage'] = percent
        chk.fix_traps()

        if not parms is None:
            chk.parameters.update(parms)
        
        print chk
        chk.save(name)
    
    def show(self, show=False, name=None):
        
        if not show and name is None:
            print 'error: nothing to show (show=False, name=None)'
            return

        print 'importing mayavi'
        import mayavi.mlab as mlab
        cr = [-0.1, +0.1]
        ct = [cr[0], 0.00, cr[1]]
        bg = (1,1,1)
        fg = (0,0,0)

        print 'creating figure'
        figure = mlab.figure(bgcolor=bg, fgcolor=fg, size=(1024, 1024))
        figure.scene.disable_render = True

        print 'creating contour3D'
        surf = mlab.contour3d(self.mesh_x, self.mesh_y, self.mesh_z,
                              self.mesh_v, contours=ct, colormap='autumn')

        print 'creating outline'
        mlab.outline(extent=[0, self.grid.px, 0, self.grid.py, 0, self.grid.pz],
                     line_width=3)

        print 'creating colorbar'
        colorbar = mlab.colorbar()
        colorbar.data_range = cr

        print 'altering colorbar properties'
        lut = surf.module_manager.scalar_lut_manager.lut.table.to_array()
        lc = (255, 255, 0, 255)
        rc = (255, 0, 0, 255)
        for i in range(4):
            lut[:   , i] = lc[i]
            lut[:200, i] = rc[i]
        surf.module_manager.scalar_lut_manager.lut.table = lut
        mlab.draw()
        colorbar.trait_set(visible=False)

        print 'altering view angles'
        azimuth, zenith, distance, focalpoint = mlab.view()
        mlab.view(distance=1.10*distance)

        print 'saving image'
        if not name is None:
            figure.scene.disable_render = False
            mlab.savefig(name)
            langmuir.plot.crop(name)

        print 'showing mayavi window'
        if show:
            figure.scene.disable_render = False
            mlab.show()        

if __name__ == '__main__':
    work = os.getcwd()

    parms = {'exciton.binding' : 0.088581606744, 'trap.potential' : 0.5}
    surface = Surface(192, 192, 32, 24, 24, 4)
    surface.evaluate_with_wave_args(langmuir.surface.gyroid)
    surface.threshold()
    
    os.chdir('/home/adam/Desktop')
    stub = 'gyroid_%dx%dx%d_r1e-4_n%dx%dx%d_g%.2f/run.%d/sim.inp'
    for g in [0.00, 0.75, 1.00, 1.25, 1.50]:
        for r in [0, 1, 2]:
            parms.update({'coulomb.gaussian.sigma' : g})
            name = stub % (surface.grid.px, surface.grid.py, surface.grid.pz,
                           surface.wave_x.n, surface.wave_y.n,
                           surface.wave_z.n, g, r)
            print name
            surface.save(name, parms=parms)
    surface.show(show=True, name='gyroid.png')