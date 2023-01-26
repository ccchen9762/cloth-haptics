#include "Deformable.h"

Deformable::Deformable(int width, int length, chai3d::cVector3d offset) : 
		m_width(width), m_length(length), m_offset(offset) {

	m_nodes = std::vector<std::vector<cGELSkeletonNode*>>(length, std::vector<cGELSkeletonNode*>(width, nullptr));

	m_defObject = new cGELMesh();
}

Deformable::~Deformable() {
	/*for (int i = 0; i < m_length; i++) {
		for (int j = 0; j < m_width; j++) {
			delete m_nodes[i][j];
		}
	}

	delete m_defObject;*/
}