try:
    import pyparsing as pyp
except ImportError:
    pass

import numpy as np
import tempfile
import vtk
import os
import re

def to_image_data(x, y, z, v):
    """
    Create vtkImageData from numpy arrays.

    :param x: x-mesh
    :param y: y-mesh
    :param z: z-mesh
    :param v: v-mesh

    :type x: np.ndarray
    :type y: np.ndarray
    :type z: np.ndarray
    :type v: np.ndarray

    >>> x, y, z = np.mgrid[0:10:10j, 0:10:10j, 0:10:10j]
    >>> v = x * x + y * y
    >>> i = to_image_data(x, y, z, v)
    """
    x0, y0, z0 = np.amin(x), np.amin(y), np.amin(z)
    x1, y1, z1 = np.amax(x), np.amax(y), np.amax(z)
    dx = abs(x[1,0,0] - x[0,0,0])
    dy = abs(y[0,1,0] - y[0,0,0])
    dz = abs(z[0,0,1] - z[0,0,0])
    px, py, pz = x.shape

    x0 = x0 - 0.5 * (x1 - x0)
    y0 = y0 - 0.5 * (y1 - y0)
    z0 = z0 - 0.5 * (z1 - z0)

    image_data = vtk.vtkImageData()
    image_data.SetDimensions(px, py, pz)
    image_data.SetExtent(0, px - 1, 0, py - 1, 0, pz - 1)
    image_data.SetOrigin(x0, y0, z0)
    image_data.SetSpacing(dx, dy, dz)

    point_data_array = vtk.vtkDoubleArray()
    point_data_array.SetNumberOfComponents(1)
    point_data_array.SetNumberOfValues(v.size)
    point_data_array.SetName('potentials')

    for i, ((xi, yi, zi), vi) in enumerate(np.ndenumerate(v)):
        point_id = image_data.ComputePointId((xi, yi, zi))
        point_data_array.SetValue(point_id, vi)

    image_data.GetPointData().SetScalars(point_data_array)

    return image_data

def save_image_data_vti(image_data, file_name='image.vti'):
    """
    Save vtkImageData to xml file that can be loaded in other programs.

    :param image_data: vtkImageData object
    :param file_name: name of output file

    :type image_data: vtkImageData
    :type file_name: str
    """
    stub, ext = os.path.splitext(file_name)
    file_name = stub + '.vti'
    writer = vtk.vtkXMLImageDataWriter()
    writer.SetFileName(file_name)
    writer.SetInput(image_data)
    writer.Write()

def save_image_data_isosurface_pov(image_data, file_name='iso.inc', level=0.1):
    """
    Save vtkImageData to povray inc file as a 'mesh2' object.  The inc file
    can be used in a povray scene to render the image data as an isosurface.

    :param image_data: vtkImageData object
    :param file_name: name of output file
    :param level: contour isosurface value

    :type image_data: vtkImageData
    :type file_name: str
    :type level: float
    """
    stub, ext = os.path.splitext(file_name)
    file_name = stub + '.inc'

    contour = vtk.vtkMarchingCubes()
    contour.SetInput(image_data)
    contour.ComputeNormalsOn()
    contour.ComputeScalarsOn()
    contour.SetNumberOfContours(1)
    contour.SetValue(0, level)

    mapper = vtk.vtkPolyDataMapper()
    mapper.SetInput(contour.GetOutput())
    mapper.ScalarVisibilityOn()

    actor = vtk.vtkLODActor()
    actor.SetNumberOfCloudPoints(1000000)
    actor.SetMapper(mapper)
    actor.GetProperty().SetColor(1, 1, 1)
    actor.GetProperty().SetOpacity(1.0)

    render = vtk.vtkRenderer()
    render.CreateLight()
    render.AddActor(actor)

    window = vtk.vtkRenderWindow()
    window.AddRenderer(render)
    window.SetSize(256, 256)

    temp_file = tempfile.mkstemp(dir=os.getcwd(), suffix='.inc')[-1]

    writer = vtk.vtkPOVExporter()
    writer.SetFileName(temp_file)
    writer.SetInput(window)
    writer.Write()

    try:
        exp = pyp.dictOf(pyp.Word(pyp.alphas, pyp.alphanums + '_'),
            pyp.originalTextFor(pyp.nestedExpr(r'{', r'}')))
        exp.ignore(pyp.Keyword('matrix') + pyp.nestedExpr(r'<', r'>'))
        exp.ignore(r'//' + pyp.restOfLine)
        results = exp.parseFile(temp_file)

        exp = pyp.Suppress('{') + exp + pyp.Suppress('}')
        results = exp.parseString(results['mesh2'])

        if 'face_indices' in results:
            regex = r'(<\s*\d+,\s*\d+,\s*\d+\s*>\s*,)\s*\d+,\s*\d+,\s*\d+\s*,?'
            regex = re.compile(regex)
            results['face_indices'] = regex.sub(r'\g<1>',
                    results['face_indices'])

        if 'texture_list' in results:
            del results['texture_list']

        if 'texture' in results:
            del results['texture']

        with open(file_name, 'w') as ohandle:
            print >> ohandle, '#declare %s =' % stub
            print >> ohandle, 'mesh2 {'
            for key in results.keys():
                print >> ohandle, key
                print >> ohandle, results[key]
            print >> ohandle, '}'

    except:
        print 'could not alter pov mesh'
        shutil.copy(temp_file, file_name)
        return
    finally:
        os.remove(temp_file)
