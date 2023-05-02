#include "Rigid.h"

#include "ChaiWorld.h"

Rigid::Rigid(double width, double length, chai3d::cVector3d offset,
    double stiffness, double staticFriction, double dynamicFriction, double textureLevel) :
    m_width(width), m_length(length), m_offset(offset),
    m_stiffness(stiffness),m_staticFriction(staticFriction), m_dynamicFriction(dynamicFriction), m_textureLevel(textureLevel) {

    // create a mesh
    m_object = new chai3d::cMesh();
}

Rigid::~Rigid() {
}

void Rigid::AttachToWorld(ChaiWorld& chaiWorld) {
    // create plane
    cCreatePlane(m_object, m_width, m_length);

    //rigid.m_object->createAABBCollisionDetector(m_toolRadius);
    m_object->createAABBCollisionDetector(chaiWorld.getMultiCursorRadius());

    // create collision detector
    //rigid.m_object->createAABBCollisionDetector(m_deviceRadius);

    // add object to world
    chaiWorld.getWorld()->addChild(m_object);

    // set the position of the object
    m_object->setLocalPos(m_offset);

    // set graphic properties
    bool fileload;
    m_object->m_texture = chai3d::cTexture2d::create();
    fileload = m_object->m_texture->loadFromFile(RESOURCE_PATH("../resources/images/brownboard.jpg"));
    if (!fileload)
    {
#if defined(_MSVC)
        fileload = m_object->m_texture->loadFromFile("../../../bin/resources/images/brownboard.jpg");
#endif
    }
    if (!fileload)
    {
        std::cout << "Error - Rigid texture image failed to load correctly." << std::endl;
        //close();
        //return (-1);
    }

    // enable texture mapping
    m_object->setUseTexture(true);
    m_object->m_material->setWhite();

    // create normal map from texture data
    chai3d::cNormalMapPtr normalMap0 = chai3d::cNormalMap::create();
    normalMap0->createMap(m_object->m_texture);
    m_object->m_normalMap = normalMap0;

    // set haptic properties
    m_object->m_material->setStiffness(m_stiffness * chaiWorld.getMaxStiffness());
    m_object->m_material->setStaticFriction(m_staticFriction);
    m_object->m_material->setDynamicFriction(m_dynamicFriction);
    m_object->m_material->setTextureLevel(m_textureLevel);
    m_object->m_material->setHapticTriangleSides(true, true);
}

