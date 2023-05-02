#include "Polygons.h"

#include "ChaiWorld.h"

Polygons::Polygons(int width, int length, chai3d::cVector3d offset,
	double stiffness, double staticFriction, double dynamicFriction, double textureLevel) :
	m_width(width), m_length(length), m_offset(offset),
	m_stiffness(stiffness), m_staticFriction(staticFriction), m_dynamicFriction(dynamicFriction), m_textureLevel(textureLevel) {

	// create a mesh
	m_object = new chai3d::cMesh();

    m_indices.resize((width-1) * (length-1) * 2 * 3);
    m_positions.resize(width * length);

    // fill in position
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < length; j++) {
            /*m_positions[i * width + j] = chai3d::cVector3d(offset.x() + ((float(i) / width) * 2 - 1) * 2.0f / 5,
                                                                 offset.y() + ((float(j) / length) * 2 - 1) * 2.0f / 5,
                                                                 offset.z());*/
            m_positions[i * width + j] = chai3d::cVector3d(offset.x() - 0.1 * length / 2 + 0.1 * (double)i,
                                                           offset.y() - 0.1 * width / 2 + 0.1 * (double)j,
                                                           offset.z());
        }
    }

    // fill in indices
    int* id = &m_indices[0];
    for (int i = 0; i < width-1; i++) {
        for (int j = 0; j < length-1; j++) {
            int i0 = i * width + j;
            int i1 = i0 + 1;
            int i2 = i0 + width;
            int i3 = i2 + 1;
            if ((j + i) % 2) {
                *id++ = i0; *id++ = i2; *id++ = i1;
                *id++ = i1; *id++ = i2; *id++ = i3;
            }
            else {
                *id++ = i0; *id++ = i2; *id++ = i3;
                *id++ = i0; *id++ = i3; *id++ = i1;
            }
        }
    }
}

Polygons::~Polygons() {
}

void Polygons::AttachToWorld(ChaiWorld& chaiWorld) {
    chaiWorld.getWorld()->addChild(m_object);

    // set the position of the object at the center of the world
    m_object->setLocalPos(0.0, 0.0, 0.0);

    // Since we want to see our polygons from both sides, we disable culling.
    m_object->setUseCulling(false);

    for (int i = 0; i < m_indices.size(); i += 3) {
        // define triangle points
        chai3d::cVector3d p0 = chai3d::cVector3d(m_positions[m_indices[i + 0]].x(),
            m_positions[m_indices[i + 0]].y(),
            m_positions[m_indices[i + 0]].z());
        chai3d::cVector3d p1 = chai3d::cVector3d(m_positions[m_indices[i + 1]].x(),
            m_positions[m_indices[i + 1]].y(),
            m_positions[m_indices[i + 1]].z());
        chai3d::cVector3d p2 = chai3d::cVector3d(m_positions[m_indices[i + 2]].x(),
            m_positions[m_indices[i + 2]].y(),
            m_positions[m_indices[i + 2]].z());

        // create three new vertices
        int vertex0 = m_object->newVertex();
        int vertex1 = m_object->newVertex();
        int vertex2 = m_object->newVertex();

        // set position of each vertex
        m_object->m_vertices->setLocalPos(vertex0, p0);
        m_object->m_vertices->setLocalPos(vertex1, p1);
        m_object->m_vertices->setLocalPos(vertex2, p2);

        // assign color to each vertex
        m_object->m_vertices->setColor(vertex0, chai3d::cColorf((p0.x() + 1.0) * 0.5,
            (p0.y() + 1.0) * 0.5,
            0.5));
        m_object->m_vertices->setColor(vertex1, chai3d::cColorf((p1.x() + 1.0) * 0.5,
            (p1.y() + 1.0) * 0.5,
            0.5));
        m_object->m_vertices->setColor(vertex2, chai3d::cColorf((p2.x() + 1.0) * 0.5,
            (p2.y() + 1.0) * 0.5,
            0.5));

        // create new triangle from vertices
        m_object->newTriangle(vertex0, vertex1, vertex2);
    }

    // compute surface normals
    m_object->computeAllNormals();

    // we indicate that we ware rendering triangles by using specific colors for each of them (see above)
    m_object->setUseVertexColors(true);

    // we indicate that we also using material properties. If you set this parameter to 'false'
    // you will notice that only vertex colors are used to render triangle, and lighting
    // will not longer have any effect.
    m_object->setUseMaterial(true);

    // compute a boundary box
    m_object->computeBoundaryBox(true);

    //polygons.m_object->createAABBCollisionDetector(m_toolRadius);
    m_object->createAABBCollisionDetector(chaiWorld.getMultiCursorRadius());

    // set haptic properties
    m_object->m_material->setStiffness(m_stiffness * chaiWorld.getMaxStiffness());
    m_object->m_material->setStaticFriction(m_staticFriction);
    m_object->m_material->setDynamicFriction(m_dynamicFriction);
    m_object->m_material->setTextureLevel(m_textureLevel);
    m_object->m_material->setHapticTriangleSides(true, true);
}

void Polygons::updatePolygons() {
    for (int i = 0; i < m_object->getNumVertices(); i += 3) {
        chai3d::cVector3d p0 = chai3d::cVector3d(m_positions[m_indices[i + 0]].x(), m_positions[m_indices[i + 0]].y(), m_positions[m_indices[i + 0]].z());
        chai3d::cVector3d p1 = chai3d::cVector3d(m_positions[m_indices[i + 1]].x(), m_positions[m_indices[i + 1]].y(), m_positions[m_indices[i + 1]].z());
        chai3d::cVector3d p2 = chai3d::cVector3d(m_positions[m_indices[i + 2]].x(), m_positions[m_indices[i + 2]].y(), m_positions[m_indices[i + 2]].z());

        m_object->m_vertices->setLocalPos(i, p0);
        m_object->m_vertices->setLocalPos(i + 1, p1);
        m_object->m_vertices->setLocalPos(i + 2, p2);
    }
}

void Polygons::changeWireMode() {
    bool useWireMode = !m_object->getWireMode();
    m_object->setWireMode(useWireMode);
    if (useWireMode)
        std::cout << "> Wire mode enabled  \r";
    else
        std::cout << "> Wire mode disabled \r";
}
