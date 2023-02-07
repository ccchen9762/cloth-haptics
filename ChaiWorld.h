#pragma once

#include "chai3d.h"
#include "GEL3D.h"

#include "Deformable.h"
#include "Rigid.h"
#include "Polygons.h"


class ChaiWorld
{
public:
	ChaiWorld();
	~ChaiWorld();
	// not copyable
	ChaiWorld(const ChaiWorld&) = delete;
	ChaiWorld& operator= (const ChaiWorld&) = delete;

	// getters
	chai3d::cWorld* getWorld() { return m_world; }
	cGELWorld* getDefWorld() { return m_defWorld; }
	chai3d::cCamera* getCamera() { return m_camera; }
	chai3d::cHapticDeviceHandler* getHandler() { return m_handler; }
	chai3d::cGenericHapticDevicePtr getHapticDevice() { return m_hapticDevice; }
	chai3d::cShapeSphere* getDevice() { return m_device; }
	double getDeviceForceScale() { return m_deviceForceScale; }
	double getWorkspaceScaleFactor() { return m_workspaceScaleFactor; }
	double getMaxStiffness() { return m_maxStiffness; }
	chai3d::cHapticDeviceInfo getHapticDeviceInfo() { return m_hapticDeviceInfo; }
	
	void attachDeformableObject(Deformable& deformable);
	void attachRigidObject(Rigid& rigid);
	void attachPolygons(Polygons& Polygons);

	void cameraMoveForward();
	void cameraMoveBack();
	void cameraMoveLeft();
	void cameraMoveRight();

	// main haptics simulation loop
	void updateHaptics(double time, Deformable* cloth, Rigid* table, Deformable* cloth2 = nullptr);
	// compute forces between tool and environment
	chai3d::cVector3d computeForce(const chai3d::cVector3d& a_cursor,
		double a_cursorRadius,
		const chai3d::cVector3d& a_spherePos,
		double a_radius,
		double a_stiffness);

	//singleton instance 
	static ChaiWorld chaiWorld;

private:

	// a world that contains all objects of the virtual environment
	chai3d::cWorld* m_world;

	// a camera to render the world in the window display
	chai3d::cCamera* m_camera;

	chai3d::cVector3d m_cameraPos;
	chai3d::cVector3d m_cameraLookAt;

	// a light source to illuminate the objects in the world
	chai3d::cDirectionalLight* m_light;

	// a haptic device handler
	chai3d::cHapticDeviceHandler* m_handler;

	// a pointer to the current haptic device
	chai3d::cGenericHapticDevicePtr m_hapticDevice;

	chai3d::cToolCursor* m_tool;

	// force scale factor
	double m_deviceForceScale;

	// scale factor between the device workspace and cursor workspace
	double m_workspaceScaleFactor;

	// properties
	double m_maxStiffness;

	// desired workspace radius of the virtual cursor
	double m_cursorWorkspaceRadius;

	// deformable world
	cGELWorld* m_defWorld;

	// haptic device information
	chai3d::cHapticDeviceInfo m_hapticDeviceInfo;

	// haptic device model
	chai3d::cShapeSphere* m_device;
	double m_deviceRadius;
};
