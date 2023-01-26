#include "Rigid.h"

#include "ChaiWorld.h"

#include <iostream>

Rigid::Rigid(double width, double length, chai3d::cVector3d offset,
    double stiffness, double staticFriction, double dynamicFriction, double textureLevel) :
    m_width(width), m_length(length), m_offset(offset),
    m_stiffness(stiffness),m_staticFriction(staticFriction), m_dynamicFriction(dynamicFriction), m_textureLevel(textureLevel) {

    // create a mesh
    m_object = new chai3d::cMesh();
}

Rigid::~Rigid() {
}