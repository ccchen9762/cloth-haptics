#include "Deformable.h"

#include "ChaiWorld.h"

Deformable::Deformable(int width, int length, chai3d::cVector3d offset,
	double elongation, double flexion, double torsion) :
		m_width(width), m_length(length), m_offset(offset),
		m_elongation(elongation), m_flexion(flexion), m_torsion(torsion),
		m_stiffness(100), m_modelRadius(0.0f), m_staticFriction(0.3), m_dynamicFriction(0.2){

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


void Deformable::AttachToWorld(ChaiWorld& chaiWorld) {

    chaiWorld.getDefWorld()->m_gelMeshes.push_front(m_defObject);

    // build dynamic vertices
    m_defObject->buildVertices();

    // set default properties for skeleton nodes
    cGELSkeletonNode::s_default_radius = 0.05;  // [m]
    cGELSkeletonNode::s_default_kDampingPos = 5.0;  // 2.5
    cGELSkeletonNode::s_default_kDampingRot = 0.6;  // 0.6
    cGELSkeletonNode::s_default_mass = 0.002; // [kg]
    cGELSkeletonNode::s_default_showFrame = false;
    cGELSkeletonNode::s_default_color.setBlueLightSky();
    cGELSkeletonNode::s_default_useGravity = true;
    cGELSkeletonNode::s_default_gravity.set(0.00, 0.00, -9.81);
    m_modelRadius = cGELSkeletonNode::s_default_radius;

    // use internal skeleton as deformable model
    m_defObject->m_useSkeletonModel = true;

    // create an array of nodes
    for (int i = 0; i < m_length; i++)
    {
        for (int j = 0; j < m_width; j++)
        {
            cGELSkeletonNode* newNode = new cGELSkeletonNode();
            m_defObject->m_nodes.push_front(newNode);
            newNode->m_pos.set((m_offset.x() - 0.1 * m_length / 2 + 0.1 * (double)i),
                (m_offset.y() - 0.1 * m_width / 2 + 0.1 * (double)j),
                m_offset.z());
            m_nodes[i][j] = newNode;
        }
    }

    // set corner nodes as fixed
    m_nodes.front().front()->m_fixed = true;
    m_nodes.front().back()->m_fixed = true;
    m_nodes.back().front()->m_fixed = true;
    m_nodes.back().back()->m_fixed = true;

    // set default physical properties for links
    cGELSkeletonLink::s_default_kSpringElongation = m_elongation;  // [N/m]
    cGELSkeletonLink::s_default_kSpringFlexion = m_flexion;   // [Nm/RAD]
    cGELSkeletonLink::s_default_kSpringTorsion = m_torsion;   // [Nm/RAD]
    cGELSkeletonLink::s_default_color.setWhite();

    // create links between nodes
    for (int i = 0; i < m_length - 1; i++)
    {
        for (int j = 0; j < m_width - 1; j++)
        {
            cGELSkeletonLink* newLinkX0 = new cGELSkeletonLink(m_nodes[i + 0][j + 0], m_nodes[i + 1][j + 0]);
            cGELSkeletonLink* newLinkX1 = new cGELSkeletonLink(m_nodes[i + 0][j + 1], m_nodes[i + 1][j + 1]);
            cGELSkeletonLink* newLinkY0 = new cGELSkeletonLink(m_nodes[i + 0][j + 0], m_nodes[i + 0][j + 1]);
            cGELSkeletonLink* newLinkY1 = new cGELSkeletonLink(m_nodes[i + 1][j + 0], m_nodes[i + 1][j + 1]);
            m_defObject->m_links.push_front(newLinkX0);
            m_defObject->m_links.push_front(newLinkX1);
            m_defObject->m_links.push_front(newLinkY0);
            m_defObject->m_links.push_front(newLinkY1);
        }
    }

    // connect skin (mesh) to skeleton (GEM)
    m_defObject->connectVerticesToSkeleton(false);

    // show/hide underlying dynamic skeleton model
    m_defObject->m_showSkeletonModel = true;
}