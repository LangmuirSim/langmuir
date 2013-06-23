# -*- coding: utf-8 -*-
import langmuir as lm
import numpy as np
import os

class System:
    def __init__(self, gridx, gridy, gridz):
        self.grid = lm.grid.Grid.vtk(0.5, 0.5, 0.5, 1, 1, 1, gridx, gridy, gridz)
        self.imap = lm.grid.IndexMapper(self.grid.px, self.grid.py, self.grid.pz)
        self.x, self.y, self.z = self.grid.create_mgrid()
        self.v = np.zeros(self.x.shape)

    def evaluate(self, func, *args, **kwargs):
        self.v = func(self.x, self.y, self.z, *args, **kwargs)

    def threshold(self, potential=0.5):
        self.v[self.v >= 0] = potential
        self.v[self.v <= 0] = 0

    def save(self, name, template=None, parms=None):
        if template is None:
            chk = lm.checkpoint.CheckPoint()
        elif isinstance(template, lm.checkpoint.CheckPoint):
            chk = template
        else:
            chk = lm.checkpoint.load(template)

        try:
            sites, values = zip(*((self.imap.indexS(x, y, z), v) for
                (x, y, z), v in np.ndenumerate(self.v) if not v == 0.0))
        except ValueError:
            sites  = []
            values = []

        volume = self.grid.px * self.grid.py * self.grid.pz

        assert len(sites) == len(values)

        if sites:
            percent = float(len(sites) + 0.5)/float(volume)
        else:
            percent = 0.0

        chk.reset()
        chk.traps = sites
        chk.trapPotentials = values
        chk.parameters['grid.x'         ] = self.grid.px
        chk.parameters['grid.y'         ] = self.grid.py
        chk.parameters['grid.z'         ] = self.grid.pz
        chk.parameters['trap.percentage'] = percent
        chk.fix_traps()

        if not parms is None:
            chk.parameters.update(parms)

        chk.save(name)

if __name__ == '__main__':
    work = os.getcwd()
    system = System(192, 192, 32)
    wavex = lm.surface.WaveDimensions(system.grid.px, 8)
    wavey = lm.surface.WaveDimensions(system.grid.py, 8)
    wavez = lm.surface.WaveDimensions(system.grid.pz, 8)
    system.evaluate(lm.surface.gyroid, wavex.k, wavey.k, wavez.k)
    system.threshold()
    system.save('gyroid.inp')