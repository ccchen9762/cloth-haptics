#pragma once

#include "Macro.h"
#include "Global.h"
#include "chai3d.h"

class Polygons
{
	friend class ChaiWorld;

public:
	Polygons(int width, int length, chai3d::cVector3d offset,
		double stiffness, double staticFriction, double dynamicFriction, double textureLevel);
	~Polygons();

	void updatePolygons();
	void changeWireMode();

private:
	chai3d::cMesh* m_object;

	chai3d::cVector3d m_offset;

	// size
	double m_width;
	double m_length;

	std::vector<int> m_indices;
	std::vector<chai3d::cVector3d> m_positions;
	//std::vector<chai3d::cVector3d> m_velocity;
	//std::vector<chai3d::cVector3d> m_acceleration;

	// texture setttings
	double m_stiffness;
	double m_staticFriction;
	double m_dynamicFriction;
	double m_textureLevel;
};



