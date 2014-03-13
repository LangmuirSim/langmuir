"""
.. note::
    Functions for working with vtk

.. moduleauthor:: Adam Gagorik <adam.gagorik@gmail.com>
"""
import langmuir as lm
import numpy as np
import vtk
import os

def create_image_data(nx, ny, nz, dx=1.0, dy=1.0, dz=1.0, ox=0, oy=0, oz=0):
    """
    Construct an empty vtkImageData.  This data structure is a regular grid
    with constant spacing.

    :param nx: number of grid points along x-axis
    :param ny: number of grid points along y-axis
    :param nz: number of grid points along z-axis
    :param dx: spacing along x-axis
    :param dy: spacing along y-axis
    :param dz: spacing along z-axis
    :param ox: origin of x-axis
    :param oy: origin of y-axis
    :param oz: origin of z-axis

    :type nx: int
    :type ny: int
    :type nz: int
    :type dx: float
    :type dy: float
    :type dz: float
    :type ox: float
    :type oy: float
    :type oz: float

    >>> image_data = create_image_data(32, 32, 32)
    """
    image_data = vtk.vtkImageData()
    image_data.SetDimensions(nx, ny, nz)
    image_data.SetExtent(0, nx - 1, 0, ny - 1, 0, nz - 1)
    image_data.SetOrigin(ox, oy, oz)
    image_data.SetSpacing(dx, dy, dz)
    return image_data

def create_image_data_from_array(array, *args, **kwargs):
    """
    Construct an empty vtkImageData and fill it with the array's values.

    :param array: data with 3 dimensions.
    :type array: :py:class:`numpy.ndarray`

    >>> image_data = create_image_data_from_array(surf)

    .. seealso:: :py:func:`create_image_data`
    """
    ndims = len(array.shape)
    nx, ny, nz = 1, 1, 1

    if ndims > 3 or ndims <= 0:
        raise ValueError, 'data dimension(%d) not supported' % ndims

    if ndims == 3:
        nx, ny, nz = array.shape

    if ndims == 2:
        nx, ny = array.shape
        nz = 1
        array = np.expand_dims(array, 2)

    if ndims == 1:
        nx, = array.shape
        ny, nz = 1, 1
        array = np.expand_dims(np.expand_dims(array, 1), 1)

    image_data = create_image_data(nx, ny, nz, **kwargs)

    if vtk.VTK_MAJOR_VERSION <= 5:
        image_data.SetNumberOfScalarComponents(1)
        image_data.SetScalarTypeToDouble()
    else:
        image_data.AllocateScalars(vtk.VTK_DOUBLE, 1)

    for i, ((xi, yi, zi), vi) in enumerate(np.ndenumerate(array)):
        image_data.SetScalarComponentFromFloat(xi, yi, zi, 0, vi)

    return image_data

def save_image_data(handle, image_data):
    """
    Save vtkImageData to XML file.

    :param handle: file name
    :param image_data: source

    :type handle: str
    :type image_data: :py:class:`vtk.vtkImageData`

    >>> image_data = create_image_data(32, 32, 32)
    >>> save_image_data('test.vit', image_data)
    """
    stub, ext = os.path.splitext(handle)
    handle = stub + '.vti'
    writer = vtk.vtkXMLImageDataWriter()
    writer.SetInput(image_data)
    writer.SetFileName(handle)
    writer.Write()

def load_image_data(handle):
    """
    Load vtkImageData from XML file.

    :param handle: file name
    :type handle: str
    """
    reader = vtk.vtkXMLImageDataReader()
    reader.SetFileName(handle)
    return reader.GetOutput()

def pipe(i, o):
    """
    Pipes one vtkObject into another.

    :param i: input
    :param o: filter

    :type i: :py:class:`vtk.vtkObject`
    :type o: :py:class:`vtk.vtkObject`

    >>> vtkImageData = create_image_data(32, 32, 32)
    >>> vtkMarchingCubes = vtk.vtkMarchingCubes()
    >>> pipe(vtkImageData, vtkMarchingCubes)
    """
    try:
        o.SetInputConnection(i.GetOutputPort())
    except AttributeError:
        if vtk.VTK_MAJOR_VERSION <= 5:
            o.SetInput(i)
        else:
            o.SetInputData(i)

def filter_marching_cubes(vtkObject, isovalue=1e-5):
    """
    Create vtkMarchingCubes filter on vtkObject.

    :param vtkObject: input
    :param isovalue: contour value

    :param vtkObject: :py:class:`vtk.vtkObject`
    :type isovalue: float

    >>> image_data = create_image_data_from_array(surf)
    >>> cubes = filter_marching_cubes(image_data)
    >>> show(cubes)
    """
    cubes = vtk.vtkMarchingCubes()
    pipe(vtkObject, cubes)

    cubes.SetValue(0, isovalue)
    cubes.ComputeScalarsOn()
    cubes.ComputeNormalsOn()

    return cubes

def filter_marching_squares(vtkObject, isovalue=1e-5):
    """
    Create vtkMarchingSquares filter on vtkObject.

    :param vtkObject: input
    :param isovalue: contour value

    :param vtkObject: :py:class:`vtk.vtkObject`
    :type isovalue: float

    >>> image_data = create_image_data_from_array(surf)
    >>> squares = filter_marching_cubes(image_data)
    >>> show(squares)
    """
    squares = vtk.vtkMarchingSquares()
    pipe(vtkObject, squares)

    squares.SetValue(0, isovalue)
    #squares.ComputeScalarsOn()
    #squares.ComputeNormalsOn()

    return squares

def filter_curvature(vtkObject, mode='gaussian'):
    """
    Create vtkCurvatures filter on vtkObject.

    :param vtkObject: input
    :param mode: gaussian or mean

    :type vtkObject: :py:class:`vtk.vtkObject`
    :type mode: str

    >>> image_data = create_image_data_from_array(surf)
    >>> cubes = filter_marching_cubes(image_data)
    >>> curve = filter_curvature(cubes)
    >>> show(curve)
    """
    curvature = vtk.vtkCurvatures()
    pipe(vtkObject, curvature)
    curvature.SetCurvatureTypeToMean()
    if mode == 'gaussian':
        curvature.SetCurvatureTypeToGaussian()
    return curvature

def show(vtkObject, w=512, h=512):
    """
    Show result of pipeline.

    :param vtkObject: anything VTK
    :param w: window width
    :param h: window height

    :type vtkObject: None
    :type w: int
    :type h: int

    >>> image_data = create_image_data_from_array(surf)
    >>> cubes = filter_marching_cubes(image_data)
    >>> show(cubes)
    """
    mapper = vtk.vtkPolyDataMapper()

    try:
        mapper.SetInputConnection(vtkObject.GetOutputPort())
    except AttributeError:
        mapper.SetInput(vtkObject)

    actor = vtk.vtkLODActor()
    actor.SetMapper(mapper)

    renderer = vtk.vtkRenderer()
    renderer.AddActor(actor)

    window = vtk.vtkRenderWindow()
    window.AddRenderer(renderer)
    window.SetSize(w, h)

    interactor = vtk.vtkRenderWindowInteractor()
    interactor.SetRenderWindow(window)

    interactor.Initialize()
    window.Render()
    interactor.Start()

if __name__ == '__main__':

    grid = lm.grid.Grid(32, 32, 32)
    wave = lm.surface.WaveDimensions(grid.lx, 2)
    surf = lm.surface.gyroid(grid.mx, grid.my, grid.mz, wave, wave, wave)

    #image_data = create_image_data_from_array(surf)
    #save_image_data(r'/home/adam/Desktop/test.vti', image_data)

    image_data = load_image_data(r'/home/adam/Desktop/test.vti')
    cubes = filter_marching_cubes(image_data)
    show(cubes)
    #curve = filter_curvature(cubes, mode='gaussian')
    #show(curve)