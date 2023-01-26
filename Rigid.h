#pragma once

#include "Macro.h"
#include "Global.h"
#include "chai3d.h"

class Rigid
{
	friend class ChaiWorld;

public:
	Rigid(double width, double length, chai3d::cVector3d offset, 
		double stiffness, double staticFriction, double dynamicFriction, double textureLevel);
	~Rigid();

	chai3d::cVector3d getOffset() { return m_offset; }

private:

	chai3d::cMesh* m_object;

	chai3d::cVector3d m_offset;

	// size
	double m_width;
	double m_length;

	// texture setttings
	double m_stiffness;
	double m_staticFriction;
	double m_dynamicFriction;
	double m_textureLevel;
};
