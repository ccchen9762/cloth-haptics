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
    m_camera->set(chai3d::cVector3d(3.0, 0.0, 0.5),    // camera position (eye)
        chai3d::cVector3d(-2.0, 0.0, 0.0),    // look at position (target)
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

    // open connection to haptic device
    m_hapticDevice->open();

    // desired workspace radius of the cursor
    m_cursorWorkspaceRadius = 0.7;

    // read the scale factor between the physical workspace of the haptic
    // device and the virtual workspace defined for the tool
    m_workspaceScaleFactor = m_cursorWorkspaceRadius / m_hapticDeviceInfo.m_workspaceRadius;

    // properties
    m_maxStiffness = m_hapticDeviceInfo.m_maxLinearStiffness / m_workspaceScaleFactor;

    // define a scale factor between the force perceived at the cursor and the
    // forces actually sent to the haptic device
    m_deviceForceScale = 5.0;

    // create a large sphere that represents the haptic device
    m_deviceRadius = 0.1;
    m_device = new chai3d::cShapeSphere(m_deviceRadius);
    m_world->addChild(m_device);
    m_device->m_material->setWhite();
    m_device->m_material->setShininess(100);


    //=================for rigid=========================
    // create a 3D tool and add it to the world
    /*m_tool = new chai3d::cToolCursor(m_world);
    m_camera->addChild(m_tool);

    // position tool in respect to camera
    m_tool->setLocalPos(0.0, 0.0, 0.0);

    // connect the haptic device to the tool
    m_tool->setHapticDevice(m_hapticDevice);

    // set radius of tool
    double toolRadius = 0.1;

    // define a radius for the tool
    m_tool->setRadius(toolRadius);

    // map the physical workspace of the haptic device to a larger virtual workspace.
    m_tool->setWorkspaceRadius(1.0);

    // haptic forces are enabled only if small forces are first sent to the device;
    // this mode avoids the force spike that occurs when the application starts when 
    // the tool is located inside an object for instance. 
    m_tool->setWaitForSmallForce(true);

    // start the haptic tool
    m_tool->start();*/



    // create a world which supports deformable object
    m_defWorld = new cGELWorld();
    m_world->addChild(m_defWorld);
}

ChaiWorld::~ChaiWorld() {
    // no need to clean on this side
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
    cGELSkeletonLink::s_default_kSpringElongation = 25.0;  // [N/m]
    cGELSkeletonLink::s_default_kSpringFlexion = 0.5;   // [Nm/RAD]
    cGELSkeletonLink::s_default_kSpringTorsion = 0.1;   // [Nm/RAD]
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

    // create collision detector
    rigid.m_object->createAABBCollisionDetector(m_deviceRadius);

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
    rigid.m_object->m_material->setHapticTriangleSides(true, false);
}

void ChaiWorld::updateHaptics(double time, Deformable* cloth, Rigid* table) {
    // read position from haptic device
    chai3d::cVector3d pos;
    m_hapticDevice->getPosition(pos);
    pos.mul(m_workspaceScaleFactor);
    m_device->setLocalPos(pos);

    // clear all external forces
    m_defWorld->clearExternalForces();

    // compute reaction forces
    chai3d::cVector3d force(0.0, 0.0, 0.0);
    for (int i = 0; i < cloth->m_length; i++)
    {
        for (int j = 0; j < cloth->m_width; j++)
        {
            chai3d::cVector3d nodePos = cloth->m_nodes[i][j]->m_pos;
            chai3d::cVector3d f = computeForce(pos, m_deviceRadius, nodePos, cloth->m_modelRadius, cloth->m_stiffness);
            chai3d::cVector3d tmpfrc = -1.0 * f;

            //X[y * 21 + x].x = nodePos.x();
            //X[y * 21 + x].y = nodePos.y() + 0.01;
            //X[y * 21 + x].z = nodePos.z();

            double modelHeight = cloth->m_modelRadius;
            //if (nodePos.get(2) - table->getOffset().z() < modelHeight)
            //    std::cout << cGELSkeletonLink::s_default_kSpringElongation * (table->getOffset().z() - nodePos.get(2)) << std::endl;
            if (nodePos.get(2) - table->getOffset().z() < modelHeight) {
                tmpfrc.z(tmpfrc.get(2) +
                    cGELSkeletonLink::s_default_kSpringElongation * (modelHeight + table->getOffset().z() - nodePos.get(2)));
            }
            cloth->m_nodes[i][j]->setExternalForce(tmpfrc);
            
            if (pos.get(2) - table->getOffset().z() < m_deviceRadius+0.02)
                f.z(f.get(2) + 0.3 * (table->getOffset().z()- pos.get(2) + m_deviceRadius+0.02));

            force.add(f);
        }
    }

    // integrate dynamics
    ChaiWorld::chaiWorld.getDefWorld()->updateDynamics(time);

    // scale force
    force.mul(ChaiWorld::chaiWorld.getDeviceForceScale() / ChaiWorld::chaiWorld.getWorkspaceScaleFactor());

    // send forces to haptic device
    ChaiWorld::chaiWorld.getHapticDevice()->setForce(force);

    /* triangle objects */
    // compute global reference frames for each object
    //ChaiWorld::chaiWorld.getWorld()->computeGlobalPositions(true);


    // for rigids

    // compute global reference frames for each object
    /*m_world->computeGlobalPositions(true);

    // update position and orientation of tool
    m_tool->updateFromDevice();

    // compute interaction forces
    m_tool->computeInteractionForces();

    // send forces to haptic device
    m_tool->applyToDevice();*/
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
    chai3d::cVector3d forceDirection = cNormalize(vSphereCursor);
    force = chai3d::cMul(penetrationDistance * a_stiffness, forceDirection);

    // return result
    return (force);
}