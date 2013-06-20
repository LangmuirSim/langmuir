# -*- coding: utf-8 -*-
"""
Created on Tue Jul  3 12:02:32 2012

@author: adam
"""

import numpy as np
import langmuir
import argparse
import os

def get_arguments(args=None):
    desc = 'create vtk files from npy arrays'
    parser = argparse.ArgumentParser(description=desc)
    parser.add_argument(dest='input', help='input file name')
    opts = parser.parse_args(args)
    return opts

def load_npy(fname):
    return np.load(fname).astype(np.float64)

def save(data, name, template=None, parms=None):

    dname = os.path.dirname(name)
    if dname and not os.path.exists(dname):
        print 'creating %s' % dname
        os.makedirs(dname)

    if template is None:
        chk = langmuir.checkpoint.CheckPoint()
        chk.parameters.set_defaults()
    else:
        chk = langmuir.checkpoint.CheckPoint(template)

    data = np.asanyarray(data)
    grid_x = data.shape[0]
    grid_y = data.shape[1]
    grid_z = data.shape[2]

    imap = langmuir.grid.IndexMapper(shape=(grid_x, grid_y, grid_z))
    sites, values = zip(*((imap.indexS(xi, yi, zi), v) for (xi, yi, zi),
                          v in np.ndenumerate(data) if v == 1))

    volume = grid_x * grid_y * grid_z
    percent = float(len(sites) + 0.5) / float(volume)

    chk.fluxState          = []
    chk.randomState        = []
    chk.electrons          = []
    chk.holes              = []
    chk.trapPotentials     = values
    chk.traps              = sites
    chk['random.seed'    ] = 0
    chk['seed.charges'   ] = 0.0
    chk['grid.x'         ] = grid_x
    chk['grid.y'         ] = grid_y
    chk['grid.z'         ] = grid_z
    chk['trap.percentage'] = percent
    chk.fix_traps()

    if not parms is None:
        chk.parameters.update(parms)

    print chk
    stub, ext = os.path.splitext(name)
    oname = stub + '.inp'
    chk.save(oname)
    print '\nsaved %s' % oname

if __name__ == '__main__':
    work = os.getcwd()
    opts = get_arguments()
    stub, ext = os.path.splitext(opts.input)

    parms = {'exciton.binding' : 0.088581606744, 'trap.potential' : 0.5,
             'coulomb.gaussian.sigma' : 1.0}

    if ext == '.npy':
        data = load_npy(opts.input)
    else:
        raise RuntimeError('unknown file type: %s' % opts.ext)

    save(data, stub, parms=parms)
