# -*- coding: utf-8 -*-
"""
Created on Fri Feb  8 08:26:00 2013

@author: adam
"""

import matplotlib.pyplot as plt
import matplotlib as mpl
import numpy as np
import langmuir
import argparse
import os

def get_arguments(args=None):
    desc = 'Analyze coulomb.dat or generate it from a text file'
    parser = argparse.ArgumentParser(description=desc)
    parser.add_argument(dest='input', help='input file name')
    parser.add_argument(dest='stub', help='output file stub', default='',
        nargs='?')
    opts = parser.parse_args(args)
    return opts

def save(handle, array, func=np.save, **kwargs):
    if isinstance(handle, str):
        handle = open(handle, 'wb')
    print 'saving %s' % handle.name
    func(handle, array, **kwargs)
    return handle

def histogram_distance(file_name, arrays, labels=None, bins=128, title=None,
                       histtype='step', normed=False, **kwargs):
    fig, ax1 = plt.subplots(1, 1)
    plt.hist(arrays, label=labels, bins=bins, histtype=histtype,
             normed=normed, **kwargs)
    plt.axhline(0, color='k', lw=2, zorder=-1000)
    plt.axvline(0, color='k', lw=2, zorder=-1000)
    plt.ticklabel_format(scilimits=(-3, 3))
    if labels:
        plt.legend(loc='best', fancybox=False, borderaxespad=1)
    plt.xlabel('r (nm)')
    plt.ylabel('counts')
    langmuir.plot.zoom()
    if title:
        plt.text(0.5, 1.1, title, transform=ax1.transAxes, size=24,
                 ha='center', va='center')
    print 'saving %s' % file_name
    langmuir.plot.save(file_name)

def histogram_energy(file_name, arrays, labels=None, bins=128, title=None,
                       histtype='step', normed=False, **kwargs):
    fig, ax1 = plt.subplots(1, 1)
    plt.hist(arrays, label=labels, bins=bins, histtype=histtype,
             normed=normed, **kwargs)
    plt.axhline(0, color='k', lw=2, zorder=-1000)
    plt.axvline(0, color='k', lw=2, zorder=-1000)
    plt.ticklabel_format(scilimits=(-3, 3))
    if labels:
        plt.legend(loc='best', fancybox=False, borderaxespad=1)
    plt.xlabel('E (eV)')
    plt.ylabel('counts')
    langmuir.plot.zoom()
    if title:
        plt.text(0.5, 1.1, title, transform=ax1.transAxes, size=24,
                 ha='center', va='center')
    print 'saving %s' % file_name
    langmuir.plot.save(file_name)

def contour_energy(file_name, energy, n=None, vmin=-1.5, vmax=1.5, zlevel=0,
                   title=None, **kwargs):
    contours = np.linspace(vmin, vmax, n)
    fig, ax1 = plt.subplots(1, 1)
    langmuir.plot.shift_subplots(horizontal=-0.05)
    cax = langmuir.plot.create_colorbar_axes(ax1, shift=0.025, width=0.03)
    x = mesh.x[:,:,zlevel]
    y = mesh.y[:,:,zlevel]
    v = energy[:,:,zlevel]
    plt.contourf(x, y, v, contours, **kwargs)
    bar = plt.colorbar(cax=cax)
    bar.locator = plt.MultipleLocator(0.2)
    bar.update_ticks()
    langmuir.plot.fix_colorbar_labels(cax)
    plt.text(1.3, 0.5, 'E (eV)', transform=ax1.transAxes, rotation=90,
             size=24, ha='center', va='center')
    plt.gca().xaxis.set_major_locator(plt.MultipleLocator(64))
    plt.gca().yaxis.set_major_locator(plt.MultipleLocator(64))
    plt.gca().xaxis.set_minor_locator(plt.MultipleLocator(32))
    plt.gca().yaxis.set_minor_locator(plt.MultipleLocator(32))
    plt.tick_params(which='both', direction='out')
    plt.xlim(grid.x0, grid.x1)
    plt.ylim(grid.y0, grid.y1)
    plt.xlabel('x (nm)')
    plt.ylabel('y (nm)')
    if title:
        plt.text(0.5, 1.1, title, transform=ax1.transAxes, size=24,
                 ha='center', va='center')
    print 'saving %s' % file_name
    langmuir.plot.save(file_name)

def save_vti(file_name, array):
    try:
        import vtk

        data_source = vtk.vtkImageData()
        data_source.SetDimensions(*grid.shape)
        data_source.SetExtent(*grid.extent)
        data_source.SetOrigin(*grid.r0)
        data_source.SetSpacing(*grid.spacing)

        point_data_array = vtk.vtkDoubleArray()
        point_data_array.SetNumberOfComponents(1)
        point_data_array.SetNumberOfValues(array.size)
        point_data_array.SetName('energy')

        for i, ((xi, yi, zi), vi) in enumerate(np.ndenumerate(array)):
            point_id = data_source.ComputePointId((xi, yi, zi))
            point_data_array.SetValue(point_id, vi)

        data_source.GetPointData().AddArray(point_data_array)

        stub, ext = os.path.splitext(file_name)
        file_name = stub + '.vti'
        writer = vtk.vtkXMLImageDataWriter()
        writer.SetFileName(file_name)
        writer.SetInput(data_source)
        writer.Write()

        print 'saved %s' % file_name
    except:
        print 'can not save vti file (no vtk module?)'

if __name__ == '__main__':

    work = os.getcwd()
    opts = get_arguments()
    chk  = langmuir.checkpoint.load(opts.input)
    grid = langmuir.grid.grid_from_checkpoint(chk)
    mesh = grid.precalculate_mesh()
    pref = langmuir.common.prefactor / 3.5
    indx = langmuir.grid.IndexMapper(grid.shape)

    os.chdir('/home/adam/Desktop')
    print 'computing electron coulomb energy'
    exids = indx.indexX(chk.electrons)
    eyids = indx.indexY(chk.electrons)
    ezids = indx.indexZ(chk.electrons)
    edist = mesh.distances(exids, eyids, ezids)
    ecoul = mesh.coulomb(exids, eyids, ezids, q=-1) * pref

    print 'computing hole coulomb energy'
    hxids = indx.indexX(chk.holes)
    hyids = indx.indexY(chk.holes)
    hzids = indx.indexZ(chk.holes)
    hdist = mesh.distances(hxids, hyids, hzids)
    hcoul = mesh.coulomb(hxids, hyids, hzids, q=+1) * pref

    print 'computing distances'
    idist = mesh.distances(exids, eyids, ezids, hxids, hyids, hzids)
    tdist = np.concatenate((edist, hdist, idist))
    tcoul = ecoul + hcoul

    save('%sedist.npy' % opts.stub, edist)
    save('%shdist.npy' % opts.stub, hdist)
    save('%sidist.npy' % opts.stub, idist)
    save('%stdist.npy' % opts.stub, idist)
    save('%secoul.npy' % opts.stub, ecoul)
    save('%shcoul.npy' % opts.stub, hcoul)
    save('%stcoul.npy' % opts.stub, tcoul)

    histogram_energy('%senergy_hist.png' % opts.stub, [ecoul.flat, hcoul.flat, tcoul.flat], ['E', 'H', 'E + H'])
    histogram_energy('%secoul_hist.png' % opts.stub, ecoul.flat, color='g', title='Electron Energy')
    histogram_energy('%shcoul_hist.png' % opts.stub, hcoul.flat, color='b', title='Hole Energy')
    histogram_energy('%stcoul_hist.png' % opts.stub, tcoul.flat, color='r', title='Total Energy')

    histogram_distance('%sdist_hist.png' % opts.stub, [edist.flat, hdist.flat, idist.flat, tdist.flat], ['EE', 'HH', 'EH', 'ALL'])
    histogram_distance('%sedist_hist.png' % opts.stub, edist.flat, color='b', title='Electron-Electron Distances')
    histogram_distance('%sidist_hist.png' % opts.stub, idist.flat, color='r', title='Electron-Hole Distances')
    histogram_distance('%shdist_hist.png' % opts.stub, hdist.flat, color='g', title='Hole-Hole Distances')
    histogram_distance('%stdist_hist.png' % opts.stub, tdist.flat, color='c', title='All Distances')

    if grid.pz == 1:
        cmap = cmap=mpl.cm.get_cmap('Paired')
        contour_energy('%secoul.png' % opts.stub, ecoul, 128, cmap=cmap,
                       title='Electron Energy')
        contour_energy('%shcoul.png' % opts.stub, hcoul, 128, cmap=cmap,
                       title='Hole Energy')
        contour_energy('%stcoul.png' % opts.stub, tcoul, 128, cmap=cmap,
                       title='Total Energy')

    save_vti('%secoul.vti' % opts.stub, ecoul)
    save_vti('%shcoul.vti' % opts.stub, hcoul)
    save_vti('%stcoul.vti' % opts.stub, tcoul)
