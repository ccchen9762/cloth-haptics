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

    // ================== Cursor properties =================
    
    // desired workspace radius of the cursor
    m_cursorWorkspaceRadius = 0.7;

    // define a scale factor between the force perceived at the cursor and the
    // forces actually sent to the haptic device
    m_deviceForceScale = 5.0;

    // define the radius of the tool (sphere)
    m_multiCursorRadius = 0.1;

    //  ============================ Cursor setup ===========================

    // create a cursor and insert into the world
    m_multiCursor = new MultiCursor(m_world, m_multiCursorRadius);
    m_world->addChild(m_multiCursor);

    // connect the haptic device to the virtual tool
    m_multiCursor->setHapticDevice(m_hapticDevice);

    // define a radius for the tool
    m_multiCursor->setRadius(m_multiCursorRadius);

    
    m_multiCursor->m_hapticPoint->m_sphereProxy->m_material->setWhite();

    // uncomment this line to see where the god object is 
    //m_multiCursor->setShowContactPoints(true, true, chai3d::cColorf(0.0, 0.0, 0.0));

    // enable if objects in the scene are going to rotate of translate
    // or possibly collide against the tool. If the environment
    // is entirely static, you can set this parameter to "false"
    m_multiCursor->enableDynamicObjects(true);

    // map the physical workspace of the haptic device to a larger virtual workspace.
    m_multiCursor->setWorkspaceRadius(m_cursorWorkspaceRadius);

    // haptic forces are enabled only if small forces are first sent to the device;
    // this mode avoids the force spike that occurs when the application starts when 
    // the tool is located inside an object for instance. 
    m_multiCursor->setWaitForSmallForce(true);

    // start the haptic tool
    m_multiCursor->start();

    // ==================== calculation ====================

    // read the scale factor between the physical workspace of the haptic
    // device and the virtual workspace defined for the tool
    m_workspaceScaleFactor = m_cursorWorkspaceRadius / m_hapticDeviceInfo.m_workspaceRadius;

    // properties same
    m_maxStiffness = m_hapticDeviceInfo.m_maxLinearStiffness / m_workspaceScaleFactor;


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
    //m_multiCursor->setLocalPos(pos); // tool side will handle position set (m_multiCursor->updateFromDevice();)

    // use proxy position to check collision with deformable object, otherwise god object will penetrate the rigidbody
    chai3d::cVector3d renderPos = m_multiCursor->getHapticPoint(0)->getGlobalPosProxy();

    // clear all external forces
    m_defWorld->clearExternalForces();

    // compute reaction forces
    chai3d::cVector3d force(0.0, 0.0, 0.0);

    for (int i = 0; i < cloth->m_length; i++)
    {
        for (int j = 0; j < cloth->m_width; j++)
        {
            chai3d::cVector3d nodePos = cloth->m_nodes[i][j]->m_pos;
            //chai3d::cVector3d f = computeForce(pos, m_multiCursorRadius, nodePos, cloth->m_modelRadius, cloth->m_stiffness);
            chai3d::cVector3d f = computeForce(renderPos, m_multiCursorRadius, nodePos, cloth->m_modelRadius, cloth->m_stiffness);
            chai3d::cVector3d tmpfrc = -1.0 * f;

            if (polygonCloth) {
                polygonCloth->m_positions[i * cloth->m_length + j].x(nodePos.x());
                polygonCloth->m_positions[i * cloth->m_length + j].y(nodePos.y());
                polygonCloth->m_positions[i * cloth->m_length + j].z(nodePos.z() + 0.04);
            }

            double modelHeight = cloth->m_modelRadius;
            //if (nodePos.get(2) - table->getOffset().z() < modelHeight)
            //    std::cout << cGELSkeletonLink::s_default_kSpringElongation * (table->getOffset().z() - nodePos.get(2)) << std::endl;
            if (nodePos.get(2) - table->getOffset().z() < modelHeight) {
                tmpfrc.z(tmpfrc.get(2) +
                    cGELSkeletonLink::s_default_kSpringElongation * (modelHeight + table->getOffset().z() - nodePos.get(2)));
            }
            cloth->m_nodes[i][j]->setExternalForce(tmpfrc);

            //force.add(f);
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


    // compute surface normals
    //polygonCloth->m_object->computeAllNormals();

    // compute a boundary box
    //polygonCloth->m_object->computeBoundaryBox(true);

    polygonCloth->m_object->createAABBCollisionDetector(m_multiCursorRadius);
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