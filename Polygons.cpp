#include "Polygons.h"

Polygons::Polygons(int width, int length, chai3d::cVector3d offset,
	double stiffness, double staticFriction, double dynamicFriction, double textureLevel) :
	m_width(width), m_length(length), m_offset(offset),
	m_stiffness(stiffness), m_staticFriction(staticFriction), m_dynamicFriction(dynamicFriction), m_textureLevel(textureLevel) {

	// create a mesh
	m_object = new chai3d::cMesh();

    m_indices.resize(width * length * 2 * 3);
    m_positions.resize((width + 1) * (length + 1));

    // fill in position
    for (int i = 0; i < width + 1; i++) {
        for (int j = 0; j < length + 1; j++) {
            /*m_positions[i * (width + 1) + j] = chai3d::cVector3d(offset.x() + ((float(i) / width) * 2 - 1) * 2.0f / 5,
                                                                 offset.y() + ((float(j) / length) * 2 - 1) * 2.0f / 5,
                                                                 offset.z());*/
            m_positions[i * (width + 1) + j] = chai3d::cVector3d(offset.x() - 0.1 * length / 2 + 0.1 * (double)i,
                                                                 offset.y() - 0.1 * width / 2 + 0.1 * (double)j,
                                                                 offset.z());
        }
    }

    // fill in indices
    int* id = &m_indices[0];
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < length; j++) {
            int i0 = i * (width + 1) + j;
            int i1 = i0 + 1;
            int i2 = i0 + (width + 1);
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