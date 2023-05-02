#include "Deformable.h"

Deformable::Deformable(int width, int length, chai3d::cVector3d offset,
	double elongation, double flexion, double torsion, 
	double c11, double c12, double c22, double c33) :
		m_width(width), m_length(length), m_offset(offset),
		m_elongation(elongation), m_flexion(flexion), m_torsion(torsion),
		m_stiffness(100), m_modelRadius(0.0f), m_staticFriction(0.3), m_dynamicFriction(0.2),
		m_c11(c11), m_c12(c12), m_c22(c22), m_c33(c33){

	m_nodes = std::vector<std::vector<cGELSkeletonNode*>>(length, std::vector<cGELSkeletonNode*>(width, nullptr));

	m_defObject = new cGELMesh();
}

/*Deformable::~Deformable() {
	for (int i = 0; i < m_length; i++) {
		for (int j = 0; j < m_width; j++) {
			delete m_nodes[i][j];
		}
	}

	delete m_defObject;
}*/