#pragma once

#include <vector>

#include "GEL3D.h"

class Deformable
{
	friend class ChaiWorld;

public:
	Deformable(int width, int length, chai3d::cVector3d offset, 
		double elongation = 25.0, double flexion = 0.5, double torsion = 0.1);
	~Deformable() = default;

	cGELMesh* getDefObject() { return m_defObject; }

	// setup object properties in world
	void AttachToWorld(ChaiWorld& chaiWorld);

private:
	int m_width;
	int m_length;

	chai3d::cVector3d m_offset;

	// object mesh
	cGELMesh* m_defObject;

	// dynamic nodes
	std::vector<std::vector<cGELSkeletonNode*>> m_nodes;

	// radius of the dynamic model sphere (GEM)
	double m_modelRadius;

	// stiffness properties between the haptic device tool and the model (GEM)
	double m_stiffness;

	// spring parameters
	double m_elongation;
	double m_flexion;
	double m_torsion;

	// deformable friction test?
	float m_staticFriction;
	float m_dynamicFriction;
};