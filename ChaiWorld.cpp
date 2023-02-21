#include "ChaiWorld.h"

#include "Macro.h"
#include "Global.h"

ChaiWorld ChaiWorld::chaiWorld;

ChaiWorld::ChaiWorld() {

    //--------------------------------------------------------------------------
    // WORLD - CAMERA - LIGHTING
    //--------------------------------------------------------------------------
    
    // create a new world.
    m_world = new chai3d::cWorld();

    // set the background color of the environment
    m_world->m_backgroundColor.setBlack();

    // create a camera and insert it into the virtual world
    m_camera = new chai3d::cCamera(m_world);
    m_world->addChild(m_camera);

    // position and orient the camera
    m_cameraPos = chai3d::cVector3d(2.5, 0.0, 0.8);
    m_cameraLookAt = chai3d::cVector3d(-2.0, 0.0, 0.0);

    m_camera->set(m_cameraPos,    // camera position (eye)
        m_cameraLookAt,    // look at position (target)
        chai3d::cVector3d(0.0, 0.0, 1.0));   // direction of the (up) vector

    // set the near and far clipping planes of the camera
    m_camera->setClippingPlanes(0.01, 10.0);    // can be bigger

    // set stereo mode
    m_camera->setStereoMode(kStereoMode);

    // set stereo eye separation and focal length (applies only if stereo is enabled)
    m_camera->setStereoEyeSeparation(0.02);
    m_camera->setStereoFocalLength(3.0);

    // set vertical mirrored display mode
    m_camera->setMirrorVertical(kMirroredDisplay);

    // enable multi-pass rendering to handle transparent objects
    m_camera->setUseMultipassTransparency(true);

    // create a directional light source
    m_light = new chai3d::cDirectionalLight(m_world);

    // insert light source inside world
    m_world->addChild(m_light);

    // enable light source
    m_light->setEnabled(true);

    // define direction of light beam
    m_light->setDir(0.0, -1.0, -1.0);

    //-----------------------------------------------------------------------
    // HAPTIC DEVICES / TOOLS
    //-----------------------------------------------------------------------

    // create a haptic device handler
    m_handler = new chai3d::cHapticDeviceHandler();

    // get access to the first available haptic device found
    m_handler->getDevice(m_hapticDevice, 0);

    // retrieve information about the current haptic device
    m_hapticDeviceInfo = m_hapticDevice->getSpecifications();

    // ================== Cursor =================
    
    // desired workspace radius of the cursor
    m_cursorWorkspaceRadius = 0.7;

    // read the scale factor between the physical workspace of the haptic
    // device and the virtual workspace defined for the tool
    m_workspaceScaleFactor = m_cursorWorkspaceRadius / m_hapticDeviceInfo.m_workspaceRadius;

    // properties same
    m_maxStiffness = m_hapticDeviceInfo.m_maxLinearStiffness / m_workspaceScaleFactor;

    // define a scale factor between the force perceived at the cursor and the
    // forces actually sent to the haptic device
    m_deviceForceScale = 5.0;

    // define the radius of the tool (sphere)
    m_multiCursorRadius = 0.1;

    // create a cursor and insert into the world
    m_multiCursor = new MultiCursor(m_world, m_multiCursorRadius);
    m_world->addChild(m_multiCursor);

    // connect the haptic device to the virtual tool
    m_multiCursor->setHapticDevice(m_hapticDevice);

    // define a radius for the tool
    m_multiCursor->setRadius(m_multiCursorRadius);

    // enable if objects in the scene are going to rotate of translate
    // or possibly collide against the tool. If the environment
    // is entirely static, you can set this parameter to "false"
    m_multiCursor->enableDynamicObjects(true);

    m_multiCursor->m_material->setWhite();
    m_multiCursor->m_material->setShininess(100);

    m_multiCursor->start();

    // ========== create a world which supports deformable object ============
    m_defWorld = new cGELWorld();
    m_world->addChild(m_defWorld);
}

ChaiWorld::~ChaiWorld() {
    // no need to clean in here
    //delete m_world;
    //delete m_camera;
    //delete m_light;
    //delete m_handler;
}

void ChaiWorld::attachDeformableObject(Deformable& deformable) {
    m_defWorld->m_gelMeshes.push_front(deformable.m_defObject);

    // build dynamic vertices
    deformable.m_defObject->buildVertices();

    // set default properties for skeleton nodes
    cGELSkeletonNode::s_default_radius = 0.05;  // [m]
    cGELSkeletonNode::s_default_kDampingPos = 5.0;  // 2.5
    cGELSkeletonNode::s_default_kDampingRot = 0.6;  // 0.6
    cGELSkeletonNode::s_default_mass = 0.002; // [kg]
    cGELSkeletonNode::s_default_showFrame = false;
    cGELSkeletonNode::s_default_color.setBlueLightSky();
    cGELSkeletonNode::s_default_useGravity = true;
    cGELSkeletonNode::s_default_gravity.set(0.00, 0.00, -9.81);
    deformable.m_modelRadius = cGELSkeletonNode::s_default_radius;

    // use internal skeleton as deformable model
    deformable.m_defObject->m_useSkeletonModel = true;

    // create an array of nodes
    for (int i = 0; i < deformable.m_length; i++)
    {
        for (int j = 0; j < deformable.m_width; j++)
        {
            cGELSkeletonNode* newNode = new cGELSkeletonNode();
            deformable.m_defObject->m_nodes.push_front(newNode);
            newNode->m_pos.set((deformable.m_offset.x() - 0.1 * deformable.m_length / 2 + 0.1 * (double)i),
                (deformable.m_offset.y() - 0.1 * deformable.m_width / 2  + 0.1 * (double)j),
                deformable.m_offset.z());
            deformable.m_nodes[i][j] = newNode;
        }
    }

    // set corner nodes as fixed
    deformable.m_nodes.front().front()->m_fixed = true;
    deformable.m_nodes.front().back()->m_fixed = true;
    deformable.m_nodes.back().front()->m_fixed = true;
    deformable.m_nodes.back().back()->m_fixed = true;

    // set default physical properties for links
    cGELSkeletonLink::s_default_kSpringElongation = deformable.m_elongation ;  // [N/m]
    cGELSkeletonLink::s_default_kSpringFlexion = deformable.m_flexion;   // [Nm/RAD]
    cGELSkeletonLink::s_default_kSpringTorsion = deformable.m_torsion;   // [Nm/RAD]
    cGELSkeletonLink::s_default_color.setWhite();

    // create links between nodes
    for (int i = 0; i < deformable.m_length-1; i++)
    {
        for (int j = 0; j < deformable.m_width-1; j++)
        {
            cGELSkeletonLink* newLinkX0 = new cGELSkeletonLink(deformable.m_nodes[i + 0][j + 0], deformable.m_nodes[i + 1][j + 0]);
            cGELSkeletonLink* newLinkX1 = new cGELSkeletonLink(deformable.m_nodes[i + 0][j + 1], deformable.m_nodes[i + 1][j + 1]);
            cGELSkeletonLink* newLinkY0 = new cGELSkeletonLink(deformable.m_nodes[i + 0][j + 0], deformable.m_nodes[i + 0][j + 1]);
            cGELSkeletonLink* newLinkY1 = new cGELSkeletonLink(deformable.m_nodes[i + 1][j + 0], deformable.m_nodes[i + 1][j + 1]);
            deformable.m_defObject->m_links.push_front(newLinkX0);
            deformable.m_defObject->m_links.push_front(newLinkX1);
            deformable.m_defObject->m_links.push_front(newLinkY0);
            deformable.m_defObject->m_links.push_front(newLinkY1);
        }
    }

    // connect skin (mesh) to skeleton (GEM)
    deformable.m_defObject->connectVerticesToSkeleton(false);

    // show/hide underlying dynamic skeleton model
    deformable.m_defObject->m_showSkeletonModel = true;
}

void ChaiWorld::attachRigidObject(Rigid& rigid) {
    // create plane
    cCreatePlane(rigid.m_object, rigid.m_width, rigid.m_length);

    //rigid.m_object->createAABBCollisionDetector(m_toolRadius);
    rigid.m_object->createAABBCollisionDetector(m_multiCursorRadius);

    // create collision detector
    //rigid.m_object->createAABBCollisionDetector(m_deviceRadius);

    // add object to world
    m_world->addChild(rigid.m_object);

    // set the position of the object
    rigid.m_object->setLocalPos(rigid.m_offset);

    // set graphic properties
    bool fileload;
    rigid.m_object->m_texture = chai3d::cTexture2d::create();
    fileload = rigid.m_object->m_texture->loadFromFile(RESOURCE_PATH("../resources/images/brownboard.jpg"));
    if (!fileload)
    {
#if defined(_MSVC)
        fileload = rigid.m_object->m_texture->loadFromFile("../../../bin/resources/images/brownboard.jpg");
#endif
    }
    if (!fileload)
    {
        std::cout << "Error - Rigid texture image failed to load correctly." << std::endl;
        //close();
        //return (-1);
    }

    // enable texture mapping
    rigid.m_object->setUseTexture(true);
    rigid.m_object->m_material->setWhite();

    // create normal map from texture data
    chai3d::cNormalMapPtr normalMap0 = chai3d::cNormalMap::create();
    normalMap0->createMap(rigid.m_object->m_texture);
    rigid.m_object->m_normalMap = normalMap0;

    // set haptic properties
    rigid.m_object->m_material->setStiffness(rigid.m_stiffness * m_maxStiffness);
    rigid.m_object->m_material->setStaticFriction(rigid.m_staticFriction);
    rigid.m_object->m_material->setDynamicFriction(rigid.m_dynamicFriction);
    rigid.m_object->m_material->setTextureLevel(rigid.m_textureLevel);
    rigid.m_object->m_material->setHapticTriangleSides(true, true);
}

void ChaiWorld::attachPolygons(Polygons& polygons) {
    
    m_world->addChild(polygons.m_object);

    // set the position of the object at the center of the world
    polygons.m_object->setLocalPos(0.0, 0.0, 0.0);

    // Since we want to see our polygons from both sides, we disable culling.
    polygons.m_object->setUseCulling(false);

    for (int i = 0; i < polygons.m_indices.size(); i += 3) {
        // define triangle points
        chai3d::cVector3d p0 = chai3d::cVector3d(polygons.m_positions[polygons.m_indices[i + 0]].x(),
                                                 polygons.m_positions[polygons.m_indices[i + 0]].y(),
                                                 polygons.m_positions[polygons.m_indices[i + 0]].z());
        chai3d::cVector3d p1 = chai3d::cVector3d(polygons.m_positions[polygons.m_indices[i + 1]].x(),
                                                 polygons.m_positions[polygons.m_indices[i + 1]].y(),
                                                 polygons.m_positions[polygons.m_indices[i + 1]].z());
        chai3d::cVector3d p2 = chai3d::cVector3d(polygons.m_positions[polygons.m_indices[i + 2]].x(),
                                                 polygons.m_positions[polygons.m_indices[i + 2]].y(),
                                                 polygons.m_positions[polygons.m_indices[i + 2]].z());

        // create three new vertices
        int vertex0 = polygons.m_object->newVertex();
        int vertex1 = polygons.m_object->newVertex();
        int vertex2 = polygons.m_object->newVertex();

        // set position of each vertex
        polygons.m_object->m_vertices->setLocalPos(vertex0, p0);
        polygons.m_object->m_vertices->setLocalPos(vertex1, p1);
        polygons.m_object->m_vertices->setLocalPos(vertex2, p2);

        // assign color to each vertex
        polygons.m_object->m_vertices->setColor(vertex0, chai3d::cColorf((p0.x() + 1.0) * 0.5,
            (p0.y() + 1.0) * 0.5,
            0.5));
        polygons.m_object->m_vertices->setColor(vertex1, chai3d::cColorf((p1.x() + 1.0) * 0.5,
            (p1.y() + 1.0) * 0.5,
            0.5));
        polygons.m_object->m_vertices->setColor(vertex2, chai3d::cColorf((p2.x() + 1.0) * 0.5,
            (p2.y() + 1.0) * 0.5,
            0.5));

        // create new triangle from vertices
        polygons.m_object->newTriangle(vertex0, vertex1, vertex2);
    }

    // compute surface normals
    polygons.m_object->computeAllNormals();

    // we indicate that we ware rendering triangles by using specific colors for each of them (see above)
    polygons.m_object->setUseVertexColors(true);

    // we indicate that we also using material properties. If you set this parameter to 'false'
    // you will notice that only vertex colors are used to render triangle, and lighting
    // will not longer have any effect.
    polygons.m_object->setUseMaterial(true);

    // compute a boundary box
    polygons.m_object->computeBoundaryBox(true);

    //polygons.m_object->createAABBCollisionDetector(m_toolRadius);
    polygons.m_object->createAABBCollisionDetector(m_multiCursorRadius);
     
    // set haptic properties
    polygons.m_object->m_material->setStiffness(polygons.m_stiffness * m_maxStiffness);
    polygons.m_object->m_material->setStaticFriction(polygons.m_staticFriction);
    polygons.m_object->m_material->setDynamicFriction(polygons.m_dynamicFriction);
    polygons.m_object->m_material->setTextureLevel(polygons.m_textureLevel);
    polygons.m_object->m_material->setHapticTriangleSides(true, true);
}

void ChaiWorld::cameraMoveLeft() {
    m_cameraPos.y(m_cameraPos.y() - 0.1);
    m_cameraLookAt.y(m_cameraLookAt.y() - 0.1);
    m_camera->set(m_cameraPos,    // camera position (eye)
        m_cameraLookAt,    // look at position (target)
        chai3d::cVector3d(0.0, 0.0, 1.0)); // up vector
}

void ChaiWorld::cameraMoveRight() {
    m_cameraPos.y(m_cameraPos.y() + 0.1);
    m_cameraLookAt.y(m_cameraLookAt.y() + 0.1);
    m_camera->set(m_cameraPos,    // camera position (eye)
        m_cameraLookAt,    // look at position (target)
        chai3d::cVector3d(0.0, 0.0, 1.0)); // up vector
}

void ChaiWorld::updateCloth(Polygons* polygonCloth) {
   
    polygonCloth->updatePolygons();
    
}

void ChaiWorld::cameraMoveForward() {
    m_cameraPos.x(m_cameraPos.x() - 0.1);
    m_cameraLookAt.x(m_cameraLookAt.x() - 0.1);
    m_camera->set(m_cameraPos,    // camera position (eye)
        m_cameraLookAt,    // look at position (target)
        chai3d::cVector3d(0.0, 0.0, 1.0)); // up vector
}

void ChaiWorld::cameraMoveBack() {
    m_cameraPos.x(m_cameraPos.x() + 0.1);
    m_cameraLookAt.x(m_cameraLookAt.x() + 0.1);
    m_camera->set(m_cameraPos,    // camera position (eye)
        m_cameraLookAt,    // look at position (target)
        chai3d::cVector3d(0.0, 0.0, 1.0)); // up vector
}

void ChaiWorld::updateHapticsMulti(double time, Rigid* table, Deformable* cloth, Polygons* polygonCloth) {
    chai3d::cVector3d pos;
    m_hapticDevice->getPosition(pos);
    pos.mul(m_workspaceScaleFactor);
    m_multiCursor->setLocalPos(pos);

    // clear all external forces
    m_defWorld->clearExternalForces();

    // compute reaction forces
    chai3d::cVector3d force(0.0, 0.0, 0.0);

    for (int i = 0; i < cloth->m_length; i++)
    {
        for (int j = 0; j < cloth->m_width; j++)
        {
            chai3d::cVector3d nodePos = cloth->m_nodes[i][j]->m_pos;
            chai3d::cVector3d f = computeForce(pos, m_multiCursorRadius, nodePos, cloth->m_modelRadius, cloth->m_stiffness);
            chai3d::cVector3d tmpfrc = -1.0 * f;

            /*if (polygonCloth) {
                polygonCloth->m_positions[i * cloth->m_length + j].x(nodePos.x());
                polygonCloth->m_positions[i * cloth->m_length + j].y(nodePos.y());
                polygonCloth->m_positions[i * cloth->m_length + j].z(nodePos.z() + 0.04);
            }*/

            double modelHeight = cloth->m_modelRadius;
            //if (nodePos.get(2) - table->getOffset().z() < modelHeight)
            //    std::cout << cGELSkeletonLink::s_default_kSpringElongation * (table->getOffset().z() - nodePos.get(2)) << std::endl;
            if (nodePos.get(2) - table->getOffset().z() < modelHeight) {
                tmpfrc.z(tmpfrc.get(2) +
                    cGELSkeletonLink::s_default_kSpringElongation * (modelHeight + table->getOffset().z() - nodePos.get(2)));
            }
            cloth->m_nodes[i][j]->setExternalForce(tmpfrc);

            force.add(f);
        }
    }

    ChaiWorld::chaiWorld.getDefWorld()->updateDynamics(time);

    // scale force
    force.mul(ChaiWorld::chaiWorld.getDeviceForceScale() / ChaiWorld::chaiWorld.getWorkspaceScaleFactor());

    // compute global reference frames for each object
    m_world->computeGlobalPositions(true);

    // update position and orientation of tool
    m_multiCursor->updateFromDevice();

    // compute interaction forces
    m_multiCursor->computeInteractionForces();

    // send forces to haptic device
    //m_multiCursor->applyToDevice();
    m_multiCursor->applyToDevice(force);

    // ====== force -> force from deformable object ===============================
    // ====== m_multiCursor->applyToDevice -> deformable force + rigid force ======
}

chai3d::cVector3d ChaiWorld::computeForce(const chai3d::cVector3d& a_cursor,
    double a_cursorRadius,
    const chai3d::cVector3d& a_spherePos,
    double a_radius,
    double a_stiffness) {

    // compute the reaction forces between the tool and the ith sphere.
    chai3d::cVector3d force;
    force.zero();
    chai3d::cVector3d vSphereCursor = a_cursor - a_spherePos;

    // check if both objects are intersecting
    if (vSphereCursor.length() < 0.0000001)
    {
        return (force);
    }

    if (vSphereCursor.length() > (a_cursorRadius + a_radius))
    {
        return (force);
    }

    // compute penetration distance between tool and surface of sphere
    double penetrationDistance = (a_cursorRadius + a_radius) - vSphereCursor.length();
    //std::cout << penetrationDistance << std::endl;
    chai3d::cVector3d forceDirection = cNormalize(vSphereCursor);
    force = chai3d::cMul(penetrationDistance * a_stiffness, forceDirection);
    // penetrationDistance * penetrationDistance * 50 * a_stiffness, forceDirection ?

    // return result
    return (force);
}