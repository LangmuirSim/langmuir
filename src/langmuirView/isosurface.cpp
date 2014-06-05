#include "isosurface.h"
#include <cmath>

#include <QDebug>

namespace MarchingCubes
{
    Triangle::Triangle(QObject *parent) :
        QObject(parent)
    {
    }

    Triangle::~Triangle()
    {
    }

    void Triangle::sort()
    {
    }

    void Triangle::setVertex(int index, QVector3D vector)
    {
        switch (index)
        {
            case 0:
            {
                v0 = vector;
                break;
            }
            case 1:
            {
                v1 = vector;
                break;
            }
            case 2:
            {
                v2 = vector;
                break;
            }
        }
    }

    void Triangle::calculateNormals()
    {
        n0 = QVector3D::normal(v1 - v0, v2 - v0);
        n1 = n0;
        n2 = n0;
    }

    Isosurface::Isosurface(QObject *parent) :
        QObject(parent), m_scalar(NULL), m_xsize(0), m_ysize(0), m_zsize(0), m_spacing(0.0), m_value(0.0)
    {
    }

    Isosurface::~Isosurface()
    {
        if (m_scalar != NULL)
        {
            delete m_scalar;
        }
        foreach(Triangle * triangle, m_triangles)
        {
            triangle->deleteLater();
        }
    }

    MarchingCubes::scalar_field& Isosurface::createScalarField(int xsize, int ysize, int zsize, float spacing)
    {
        if (m_scalar == NULL) {
            m_scalar = new MarchingCubes::scalar_field;
        }
        m_spacing = spacing;
        m_xsize   = xsize;
        m_ysize   = ysize;
        m_zsize   = zsize;

        m_scalar->resize(boost::extents[m_xsize][m_ysize][m_zsize]);

        for (int i = 0; i < m_xsize; i++)
        {
            for (int j = 0; j < m_ysize; j++)
            {
                for (int k = 0; k < m_zsize; k++)
                {
                    (*m_scalar)[i][j][k] = 0.0;
                }
            }
        }

        return *m_scalar;
    }

    void Isosurface::setIsoValue(float value)
    {
        m_value = value;
    }

    scalar_field* Isosurface::getScalarField()
    {
        return m_scalar;
    }

    void Isosurface::generate()
    {
        if (m_scalar == NULL)
        {
            qFatal("langmuir: must set scalar field");
        }

        emit progress(0);

        int count = 0;
        float vol = m_xsize * m_ysize * m_zsize;

        for (int i = 0; i < m_xsize - 1; i++)
        {
            for (int j = 0; j < m_ysize - 1; j++)
            {
                for (int k = 0; k < m_zsize - 1; k++)
                {
                    marchingCubes(i, j, k);
                    count += 1;
                    emit progress(int(100.0 * count / vol));
                }
            }
        }

        emit progress(100);

        simplify();

        emit done();
    }

    const QVector<float>& Isosurface::vertices() const
    {
        return m_vertices;
    }

    const QVector<float>& Isosurface::normals() const
    {
        return m_normals;
    }

    const QVector<unsigned int> &Isosurface::indices() const
    {
        return m_indices;
    }

    void Isosurface::clear()
    {
        m_spacing = 0.0;
        m_value   = 0.0;
        m_xsize   = 0;
        m_ysize   = 0;
        m_zsize   = 0;

        if (m_scalar != NULL)
        {
            m_scalar->resize(boost::extents[m_xsize][m_ysize][m_zsize]);
        }

        foreach(Triangle * triangle, m_triangles)
        {
            triangle->deleteLater();
        }
        m_triangles.clear();

        m_vertices.clear();
        m_normals.clear();
        m_indices.clear();
    }

    void Isosurface::marchingCubes(int xi, int yi, int zi)
    {
        scalar_field &scalar = *m_scalar;
        static float afCubeValue[8];

        // copy values of cube to local memory
        afCubeValue[0] = scalar[xi + 0][yi + 0][zi + 0];
        afCubeValue[1] = scalar[xi + 1][yi + 0][zi + 0];
        afCubeValue[2] = scalar[xi + 1][yi + 1][zi + 0];
        afCubeValue[3] = scalar[xi + 0][yi + 1][zi + 0];

        afCubeValue[4] = scalar[xi + 0][yi + 0][zi + 1];
        afCubeValue[5] = scalar[xi + 1][yi + 0][zi + 1];
        afCubeValue[6] = scalar[xi + 1][yi + 1][zi + 1];
        afCubeValue[7] = scalar[xi + 0][yi + 1][zi + 1];

        // find which vertices are inside surface
        int iFlagIndex = 0;
        for(int i = 0; i < 8; i++)
        {
            if(afCubeValue[i] <= m_value)
            {
                iFlagIndex |= 1 << i;
            }
        }

        // find which edges are intersected by the surface
        int iEdgeFlags = aiCubeEdgeFlags[iFlagIndex];

        // if the cube is entirely inside or outside of the surface, then there will be no intersections
        if(iEdgeFlags == 0)
        {
            return;
        }

        // find the point of intersection of the surface with each edge
        // find the normal to the surface at those points
        QVector<QVector3D> edges(12);
        float xv = m_spacing * xi;
        float yv = m_spacing * yi;
        float zv = m_spacing * zi;
        for(int i = 0; i < 12; i++)
        {
            //if there is an intersection on this edge
            if(iEdgeFlags & (1 << i))
            {
                float offset = getOffset(afCubeValue[a2iEdgeConnection[i][0]], afCubeValue[a2iEdgeConnection[i][1]], m_value);

                edges[i].setX(xv + (a2fVertexOffset[a2iEdgeConnection[i][0]][0]  +  offset * a2fEdgeDirection[i][0]) * m_spacing);
                edges[i].setY(yv + (a2fVertexOffset[a2iEdgeConnection[i][0]][1]  +  offset * a2fEdgeDirection[i][1]) * m_spacing);
                edges[i].setZ(zv + (a2fVertexOffset[a2iEdgeConnection[i][0]][2]  +  offset * a2fEdgeDirection[i][2]) * m_spacing);
            }
        }

        // draw the triangles that were found.  There can be up to five per cube
        for(int iTriangle = 0; iTriangle < 5; iTriangle++)
        {
            if(a2iTriangleConnectionTable[iFlagIndex][3*iTriangle] < 0)
            {
                break;
            }

            Triangle* triangle = new Triangle;

            for(int iCorner = 0; iCorner < 3; iCorner++)
            {
                int iVertex = a2iTriangleConnectionTable[iFlagIndex][3 * iTriangle + iCorner];
                triangle->setVertex(iCorner, edges.at(iVertex));
            }

            triangle->calculateNormals();
            m_triangles.push_back(triangle);
        }
    }

    // find the approximate point of intersection of the surface between two points with the values fValue1 and fValue2
    float Isosurface::getOffset(const float &v1, const float &v2, const float &v)
    {
            double fDelta = v2 - v1;
            if (fDelta == 0.0)
            {
                return 0.5;
            }
            return (v - v1) / fDelta;
    }

    void Isosurface::simplify()
    {
        emit progress(0);

        m_vertices.clear();
        m_indices.clear();
        m_normals.clear();

        m_vertices.reserve(9 * m_triangles.size());
        m_normals.reserve(9 * m_triangles.size());
        m_indices.reserve(3 * m_triangles.size());

        int index = 0;
        int count = 0;
        foreach(Triangle *triangle, m_triangles)
        {
            m_vertices.push_back(triangle->v0.x());
            m_vertices.push_back(triangle->v0.y());
            m_vertices.push_back(triangle->v0.z());

            m_normals.push_back(triangle->n0.x());
            m_normals.push_back(triangle->n0.y());
            m_normals.push_back(triangle->n0.z());

            m_indices.push_back(index);
            index += 1;

            m_vertices.push_back(triangle->v1.x());
            m_vertices.push_back(triangle->v1.y());
            m_vertices.push_back(triangle->v1.z());

            m_normals.push_back(triangle->n1.x());
            m_normals.push_back(triangle->n1.y());
            m_normals.push_back(triangle->n1.z());

            m_indices.push_back(index);
            index += 1;

            m_vertices.push_back(triangle->v2.x());
            m_vertices.push_back(triangle->v2.y());
            m_vertices.push_back(triangle->v2.z());

            m_normals.push_back(triangle->n2.x());
            m_normals.push_back(triangle->n2.y());
            m_normals.push_back(triangle->n2.z());

            m_indices.push_back(index);
            index += 1;

            count += 1;
            emit progress(int(count/float(m_triangles.size())));
        }

        foreach(Triangle * triangle, m_triangles)
        {
            triangle->deleteLater();
        }
        m_triangles.clear();

        emit progress(100);
//        QList<QVector3D> v;
//        QList<QVector3D> n;

//        emit progress(0);

//        m_indices.clear();
//        m_indices.reserve(3 * m_triangles.size());

//        int count = 0;
//        foreach(Triangle *triangle, m_triangles)
//        {
//            int i0 = v.indexOf(triangle->v0);
//            int i1 = v.indexOf(triangle->v1);
//            int i2 = v.indexOf(triangle->v2);

//            if (i0 >= 0) {
//                m_indices.push_back(i0);
//            } else {
//                v.push_back(triangle->v0);
//                n.push_back(triangle->n0);
//                m_indices.push_back(v.size() - 1);
//            }

//            if (i1 >= 0) {
//                m_indices.push_back(i1);
//            } else {
//                v.push_back(triangle->v1);
//                n.push_back(triangle->n1);
//                m_indices.push_back(v.size() - 1);
//            }

//            if (i2 >= 0) {
//                m_indices.push_back(i2);
//            } else {
//                v.push_back(triangle->v2);
//                n.push_back(triangle->n2);
//                m_indices.push_back(v.size() - 1);
//            }
//            count += 1;
//            emit progress(int(100.0 * count/float(m_triangles.size())));
//        }

//        m_vertices.clear();
//        m_vertices.reserve(3 * v.size());

//        m_normals.clear();
//        m_normals.reserve(3 * n.size());

//        count = 0;
//        foreach(QVector3D vector, v)
//        {
//            m_vertices.push_back(vector.x());
//            m_vertices.push_back(vector.y());
//            m_vertices.push_back(vector.z());
//            emit progress(int(100.0 * count/float(v.size())));
//        }

//        count = 0;
//        foreach(QVector3D normal, n)
//        {
//            m_normals.push_back(normal.x());
//            m_normals.push_back(normal.y());
//            m_normals.push_back(normal.z());
//            emit progress(int(100.0 * count/float(v.size())));
//        }

//        foreach(Triangle * triangle, m_triangles)
//        {
//            triangle->deleteLater();
//        }
//        m_triangles.clear();

//        emit progress(100);
    }
}
