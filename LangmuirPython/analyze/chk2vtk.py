# -*- coding: utf-8 -*-
"""
Created on Mon Feb  4 08:39:33 2013

@author: adam
"""

import numpy as np
import langmuir
import argparse
import vtk
import os

desc = \
"""
Creates vtk files for visualization using the data found in checkpoint file.
Use a program such as paraview or mayavi to create contour isosurfaces of
the vtu files, and glyphs of the vti files.
"""

def get_arguments(args=None):
    parser = argparse.ArgumentParser(description=desc)
    parser.add_argument(dest='input', help='input file name')
    parser.add_argument('--stub', default='', type=str,
                        help='output file name stub')
    opts = parser.parse_args(args)
    return opts

if __name__ == '__main__':

    work = os.getcwd()
    opts = get_arguments()
    chk  = langmuir.checkpoint.load(opts.input)
    grid = langmuir.grid.grid_from_checkpoint(chk)

    if chk.traps:
        print 'found traps'

        print 'mapping indices'
        if chk.trapPotentials:
            xyzv = langmuir.grid.XYZV(grid, chk.traps, chk.trapPotentials)

        elif chk.parameters.has_key('trap.potential'):
            xyzv = langmuir.grid.XYZV(grid, chk.traps, chk['trap.potential'])

        else:
            xyzv = langmuir.grid.XYZV(grid, chk.traps)

        print 'creating vtk objects'
        data_source = vtk.vtkImageData()
        data_source.SetDimensions(*grid.shape)
        data_source.SetExtent(*grid.extent)
        data_source.SetOrigin(*grid.r0)
        data_source.SetSpacing(*grid.spacing)

        point_data_array = vtk.vtkDoubleArray()
        point_data_array.SetNumberOfComponents(1)
        point_data_array.SetNumberOfValues(xyzv.mesh_v.size)
        point_data_array.SetName('potentials')

        for i, ((xi, yi, zi), vi) in enumerate(np.ndenumerate(xyzv.mesh_v)):
            point_id = data_source.ComputePointId((xi, yi, zi))
            point_data_array.SetValue(point_id, vi)

        data_source.GetPointData().AddArray(point_data_array)

        file_name = '%straps.vti' % opts.stub
        writer = vtk.vtkXMLImageDataWriter()
        writer.SetFileName(file_name)
        writer.SetInput(data_source)
        writer.Write()

        print 'saved %s\n' % file_name

    if chk.electrons:
        print 'found electrons'

        print 'mapping indices'
        xyzv = langmuir.grid.XYZV(grid, chk.electrons, -1)

        print 'creating vtk objects'
        data_source = vtk.vtkUnstructuredGrid()

        points = vtk.vtkPoints()
        points_array = vtk.vtkDoubleArray()
        points_array.SetNumberOfComponents(3)
        points_array.SetNumberOfValues(xyzv.xyz.size)

        for i, value in enumerate(xyzv.xyz):
            points_array.SetValue(i, value)

        points.SetData(points_array)
        data_source.SetPoints(points)

        cells = vtk.vtkCellArray()
        for i in range(xyzv.site_ids.size):
            cells.InsertNextCell(1)
            cells.InsertCellPoint(i)

        data_source.SetCells(1, cells)

        file_name = '%selecs.vtu' % opts.stub
        writer = vtk.vtkXMLUnstructuredGridWriter()
        writer.SetFileName(file_name)
        writer.SetInput(data_source)
        writer.Write()

        print 'saved %s\n' % file_name

    if chk.holes:
        print 'found holes'

        print 'mapping indices'
        xyzv = langmuir.grid.XYZV(grid, chk.holes, -1)

        print 'creating vtk objects'
        data_source = vtk.vtkUnstructuredGrid()

        points = vtk.vtkPoints()
        points_array = vtk.vtkDoubleArray()
        points_array.SetNumberOfComponents(3)
        points_array.SetNumberOfValues(xyzv.xyz.size)

        for i, value in enumerate(xyzv.xyz):
            points_array.SetValue(i, value)

        points.SetData(points_array)
        data_source.SetPoints(points)

        cells = vtk.vtkCellArray()
        for i in range(xyzv.site_ids.size):
            cells.InsertNextCell(1)
            cells.InsertCellPoint(i)

        data_source.SetCells(1, cells)

        file_name = '%sholes.vtu' % opts.stub
        writer = vtk.vtkXMLUnstructuredGridWriter()
        writer.SetFileName(file_name)
        writer.SetInput(data_source)
        writer.Write()

        print 'saved %s\n' % file_name
