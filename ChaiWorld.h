#pragma once

#include "chai3d.h"
#include "GEL3D.h"

#include "MultiCursor.h"
#include "Deformable.h"
#include "Rigid.h"
#include "Polygons.h"

// a singleton class to handle all chai3d stuff

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
	MultiCursor* getCursor() { return m_multiCursor; }
	double getDeviceForceScale() { return m_deviceForceScale; }
	double getWorkspaceScaleFactor() { return m_workspaceScaleFactor; }
	double getMaxStiffness() { return m_maxStiffness; }
	double getMultiCursorRadius() { return m_multiCursorRadius; }
	chai3d::cHapticDeviceInfo getHapticDeviceInfo() { return m_hapticDeviceInfo; }

	void cameraMoveForward();
	void cameraMoveBack();
	void cameraMoveLeft();
	void cameraMoveRight();

	void updateCloth(Polygons* polygonCloth);

	// main haptics simulation loop
	void updateHaptics(double time, Deformable* cloth, Rigid* table, Deformable* cloth2 = nullptr, Polygons* polygonCloth = nullptr) {};

	void updateHapticsRigid(double time, Rigid* table, Deformable* cloth, Polygons* polygonCloth) {};

	void updateHapticsMulti(double time, Rigid* table, Deformable* cloth, Polygons* polygonCloth);


	// compute forces between tool and environment
	chai3d::cVector3d computeForce(const chai3d::cVector3d& a_cursor,
		double a_cursorRadius,
		const chai3d::cVector3d& a_spherePos,
		double a_radius,
		double a_stiffness);

	// singleton instance 
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

	// force scale factor
	double m_deviceForceScale;

	// scale factor between the device workspace and cursor workspace
	double m_workspaceScaleFactor;

	// properties
	double m_maxStiffness;

	// desired workspace radius of the virtual cursor
	double m_cursorWorkspaceRadius;

	// ==============================================================
	
	// deformable world
	cGELWorld* m_defWorld;

	// haptic device information
	chai3d::cHapticDeviceInfo m_hapticDeviceInfo;

	// a cursor that can touch both deformable(cGELMesh) and rigidbody(cMesh)
	MultiCursor* m_multiCursor;
	double m_multiCursorRadius;
};
