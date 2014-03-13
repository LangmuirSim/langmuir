# -*- coding: utf-8 -*-
"""
@author: adam
"""
import langmuir as lm
import numpy as np
import unittest
import random
import os

work = os.getcwd()

test_dat1 = os.path.join(
    work, 'system.0', 'run.0', 'sim.0', 'part.0', 'out.dat')
test_dat2 = os.path.join(
    work, 'system.0', 'run.0', 'sim.0', 'part.1', 'out.dat')

test_prm1 = os.path.join(
    work, 'system.0', 'run.0', 'sim.0', 'part.0', 'out.parm')
test_prm2 = os.path.join(
    work, 'system.0', 'run.0', 'sim.0', 'part.1', 'out.parm')

test_chk1 = os.path.join(
    work, 'system.0', 'run.0', 'sim.0', 'part.0', 'out.chk')
test_chk2 = os.path.join(
    work, 'system.0', 'run.0', 'sim.0', 'part.1', 'out.chk')

class TestDatfile(unittest.TestCase):
    def test_load(self):
        lm.datfile.load(test_dat1)

class TestAnalyze(unittest.TestCase):
    def setUp(self):
        self.dat0 = lm.datfile.load(test_dat1)
        self.dat1 = lm.datfile.load(test_dat2)

    def test_combine(self):
        lm.analyze.combine([self.dat0, self.dat1])

    def test_calculate(self):
        lm.analyze.calculate(self.dat0)
        lm.analyze.calculate(self.dat1)

    def test_equilibrate(self):
        self.dat0 = lm.analyze.calculate(self.dat0)
        lm.analyze.equilibrate(self.dat0, -1, -2)

class TestParameters(unittest.TestCase):
    def setUp(self):
        self.parm = lm.parameters.load(test_prm1)

    def test_save(self):
        self.parm.save('test.save')
        os.remove('test.save')

    def test_to_ndarray(self):
        self.parm.to_ndarray()

    def test_to_series(self):
        self.parm.to_series()

    def test_to_dict(self):
        self.parm.to_dict()

    def test_compare(self):
        parm1 = lm.parameters.Parameters()
        self.assertTrue(lm.parameters.compare(parm1, self.parm))

class TestCheckPoint(unittest.TestCase):
    def setUp(self):
        self.chk = lm.checkpoint.CheckPoint()

    def test_load(self):
        self.chk = lm.checkpoint.load(test_chk1)

    def test_save(self):
        self.chk.save('test.save')
        os.remove('test.save')

    def test_clear(self):
        self.chk.clear()

    def test_compare(self):
        chk1 = lm.checkpoint.load(test_chk1)
        chk2 = lm.checkpoint.load(test_chk2)
        self.assertTrue(lm.checkpoint.compare(chk1, chk1)['valid'])
        self.assertTrue(lm.checkpoint.compare(chk2, chk2)['valid'])
        self.assertTrue(lm.checkpoint.compare(chk1, chk2)['invalid'])

    def test_fix_traps(self):
        self.chk.traps = []
        self.chk.fix_traps()

class TestFind(unittest.TestCase):
    def test_systems(self):
        systems = lm.find.systems(work)
        self.assertEquals(len(systems), 1)

    def test_runs(self):
        runs = lm.find.runs(work, 1)
        self.assertEquals(len(runs), 2)

    def test_sims(self):
        sims = lm.find.sims(work)
        self.assertEquals(len(sims), 6)

    def test_parts(self):
        parts = lm.find.parts(work, 1)
        self.assertEquals(len(parts), 10)

class TestGrid(unittest.TestCase):
    def test_grid(self):
        nx = np.random.randint(2, 32)
        ny = np.random.randint(2, 32)
        nz = np.random.randint(2, 32)
        lm.grid.Grid(nx, ny, nz)

    def test_imap(self):
        for i in range(1000):
            xsize = random.randint(1, 1024)
            ysize = random.randint(1, 1024)
            zsize = random.randint(1, 1024)
            grid = lm.grid.Grid(xsize, ysize, zsize)
            imap = lm.grid.IndexMapper(grid)

            xi = random.randint(0, xsize - 1)
            yi = random.randint(0, ysize - 1)
            zi = random.randint(0, zsize - 1)
            si = imap.indexS(xi, yi, zi)

            self.assertEquals(imap.indexS(xi, yi, zi), si)
            self.assertEquals(imap.indexX(si), xi)
            self.assertEquals(imap.indexY(si), yi)
            self.assertEquals(imap.indexZ(si), zi)

            si = random.randint(0, grid.size - 1)
            xi = imap.indexX(si)
            yi = imap.indexY(si)
            zi = imap.indexZ(si)
            self.assertEquals(imap.indexS(xi, yi, zi), si)

    def test_create_mgrid(self):
        xsize = random.randint(2, 100)
        ysize = random.randint(2, 100)
        zsize = 2
        grid  = lm.grid.Grid(xsize, ysize, zsize)

        self.assertEqual(grid.mx.shape[0], grid.nx)
        self.assertEqual(grid.mx.shape[1], grid.ny)
        self.assertEqual(grid.mx.shape[2], grid.nz)

        self.assertEqual(grid.my.shape[0], grid.nx)
        self.assertEqual(grid.my.shape[1], grid.ny)
        self.assertEqual(grid.my.shape[2], grid.nz)

        self.assertEqual(grid.mz.shape[0], grid.nx)
        self.assertEqual(grid.mz.shape[1], grid.ny)
        self.assertEqual(grid.mz.shape[2], grid.nz)

        self.assertEqual(abs(grid.mx[1,0,0] - grid.mx[0,0,0]), grid.dx)
        self.assertEqual(abs(grid.my[0,1,0] - grid.my[0,0,0]), grid.dy)
        self.assertEqual(abs(grid.mz[0,0,1] - grid.mz[0,0,0]), grid.dz)

        self.assertEqual(grid.ox.size, grid.nx)
        self.assertEqual(grid.oy.size, grid.ny)
        self.assertEqual(grid.oz.size, grid.nz)

        self.assertEqual(abs(grid.ox[1] - grid.ox[0]), grid.dx)
        self.assertEqual(abs(grid.oy[1] - grid.oy[0]), grid.dy)
        self.assertEqual(abs(grid.oz[1] - grid.oz[0]), grid.dz)

    def test_xyzv(self):
        chk  = lm.checkpoint.load(test_chk1)
        grid = lm.grid.Grid.from_checkpoint(chk)
        lm.grid.XYZV(grid, chk.electrons, -1)

    def test_mesh(self):
        grid = lm.grid.Grid(3, 4, 5)
        mesh = lm.grid.PrecalculatedMesh(grid)

        def r2(xi, yi, zi, xf, yf, zf):
            x = (grid.mx[xf,yf,zf] - grid.mx[xi,yi,zi])**2
            y = (grid.my[xf,yf,zf] - grid.my[xi,yi,zi])**2
            z = (grid.mz[xf,yf,zf] - grid.mz[xi,yi,zi])**2
            return x + y + z

        for i in range(grid.nx):
            for j in range(grid.ny):
                for k in range(grid.nz):
                    for q in range(grid.nx):
                        for r in range(grid.ny):
                            for s in range(grid.nz):
                                dx = abs(q - i)
                                dy = abs(r - j)
                                dz = abs(s - k)
                                r2_1 = r2(i, j, k, q, r, s)
                                r2_2 = mesh.r2[dx, dy, dz]
                                r1_1 = np.sqrt(r2_1)
                                r1_2 = mesh.r1[dx, dy, dz]
                                self.assertAlmostEquals(r2_1, r2_2)
                                self.assertAlmostEquals(r1_1, r1_2)

        mesh.coulomb(0, 0, 0, 1, 1, 1, q=-1)
        mesh.distances([0,1], [0,1], [0,1], 1, 1, 1)

class TestFit(unittest.TestCase):
    def test_linear_fit(self):
        x = np.linspace(-10, 10, 100)
        m = float(random.randint(1, 1000))
        b = float(random.randint(1, 1000))
        y = m * x + b
        y = y + np.random.normal(0, 0.1*m, y.size)
        f = lm.fit.FitLinear(x, y)
        self.assertAlmostEquals(f.m/m, 1.0, 0)
        self.assertAlmostEquals(f.b/b, 1.0, 0)

    def test_fit_power(self):
        x = np.linspace(-10, 10, 100)
        y = 4.12 * x ** 2 + 3.45 * x + 1.23
        y = y + np.random.normal(0, 0.05, y.size)
        f = lm.fit.FitPower(x, y, 2)
        self.assertAlmostEquals(f.popt[0], 4.12, 1)
        self.assertAlmostEquals(f.popt[1], 3.45, 1)
        self.assertAlmostEquals(f.popt[2], 1.23, 1)

class TestSurface(unittest.TestCase):
    def test_FFT(self):
        grid = lm.grid.Grid(32, 32, 32)
        wave = lm.surface.WaveDimensions(grid.lx, 3)
        surf = lm.surface.sin_x(grid.mx, grid.my, grid.mz, wave, wave, wave)
        lm.surface.FFT3D(grid.mx, grid.my, grid.mz, surf)

class TestCommon(unittest.TestCase):
    def test_tail(self):
        tokens = lm.common.tail(test_dat1).strip().split()
        self.assertEquals(tokens[0], '10')
        self.assertEquals(tokens[-1], '100')

if __name__ == '__main__':
    print 'running tests.'
    unittest.main()