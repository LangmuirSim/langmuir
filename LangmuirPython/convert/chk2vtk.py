# -*- coding: utf-8 -*-
"""
chk2vtk.py
==========

.. argparse::
    :module: chk2vtk
    :func: create_parser
    :prog: chk2vtk.py

.. moduleauthor:: Adam Gagorik <adam.gagorik@gmail.com>
"""
import langmuir as lm
import numpy as np
import argparse
import vtk
import os

desc = \
"""
Create vtk files for visualization using data in checkpoint file.
Use a program such as paraview or mayavi to create contour isosurfaces of
the vtu files, and glyphs of the vti files.
"""

def create_parser():
    parser = argparse.ArgumentParser(description=desc)
    parser.add_argument(dest='input', help='input file name')
    parser.add_argument('--stub', default='', type=str,
                        help='output file name stub')
    return parser

def get_arguments(args=None):
    parser = create_parser()
    opts = parser.parse_args(args)
    return opts

if __name__ == '__main__':

    work = os.getcwd()
    opts = get_arguments()
    chk  = lm.checkpoint.load(opts.input)
    grid = lm.grid.Grid.from_checkpoint(chk)

    if chk.traps:
        print 'found traps'
        print 'mapping indices'
        if chk.potentials:
            xyzv = lm.grid.XYZV(grid, chk.traps, chk.potentials)

        elif chk.parameters.has_key('trap.potential'):
            xyzv = lm.grid.XYZV(grid, chk.traps, chk['trap.potential'])

        else:
            xyzv = lm.grid.XYZV(grid, chk.traps)

        print 'creating vtk objects'
        data_source = vtk.vtkImageData()
        data_source.SetDimensions(*grid.shape)
        data_source.SetExtent(*grid.extent)
        data_source.SetOrigin(*grid.origin)
        data_source.SetSpacing(*grid.spacing)

        point_data_array = vtk.vtkDoubleArray()
        point_data_array.SetNumberOfComponents(1)
        point_data_array.SetNumberOfValues(xyzv.mv.size)
        point_data_array.SetName('potentials')

        for i, ((xi, yi, zi), vi) in enumerate(np.ndenumerate(xyzv.mv)):
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
        xyzv = lm.grid.XYZV(grid, chk.electrons, -1)

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
        for i in range(xyzv.s.size):
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
        xyzv = lm.grid.XYZV(grid, chk.holes, 1)

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
        for i in range(xyzv.s.size):
            cells.InsertNextCell(1)
            cells.InsertCellPoint(i)

        data_source.SetCells(1, cells)

        file_name = '%sholes.vtu' % opts.stub
        writer = vtk.vtkXMLUnstructuredGridWriter()
        writer.SetFileName(file_name)
        writer.SetInput(data_source)
        writer.Write()

        print 'saved %s\n' % file_name