//------------------------------------------------------------------------------
// DECLARED GRAPHICS VARIABLES
//------------------------------------------------------------------------------

#include "cloth.h"

//------------------------------------------------------------------------------
// DECLARED GRAPHICS VARIABLES
//------------------------------------------------------------------------------

// grid size
int numX = 20, numY = 20;
size_t total_points = (numX + 1) * (numY + 1);
float fullsize = 4.0f;
float halfsize = fullsize / 2.0f;

// FPS
float timeStep = 1 / 120.0f;
float currentTime = 0;
double accumulator = timeStep;
int selected_index = -1;
bool bDrawPoints = false;

std::vector<GLushort> indices;
std::vector<Spring> springs;

std::vector<glm::vec3> X;
std::vector<glm::vec3> V;
std::vector<glm::vec3> F;

int oldX = 0, oldY = 0;
float rX = 15, rY = 0;
int state = 1;
float dist = -15;
const int GRID_SIZE = 10;

const int STRUCTURAL_SPRING = 0;
const int SHEAR_SPRING = 1;
const int BEND_SPRING = 2;
int spring_count = 0;

const float DEFAULT_DAMPING = -0.1f;
float   KsStruct = 0.5f, KdStruct = -0.25f;
float   KsShear = 0.5f, KdShear = -0.25f;
float   KsBend = 0.85f, KdBend = -0.25f;
glm::vec3 gravity = glm::vec3(0.0f, -0.00981f, 0.0f);
float mass = 0.5f;

GLint viewport[4];
GLdouble MV[16];
GLdouble P[16];

glm::vec3 Up = glm::vec3(0, 1, 0), Right, viewDir;

LARGE_INTEGER frequency;        // ticks per second
LARGE_INTEGER t1, t2;           // ticks
double frameTimeQP = 0;
float frameTime = 0;

float startTime = 0, fps = 0;
int totalFrames = 0;

char info[MAX_PATH] = { 0 };

int iStacks = 30;
int iSlices = 30;
float fRadius = 1;

// Resolve constraint in object space
glm::vec3 center = glm::vec3(0, 0, 0); //object space center of ellipsoid
//float radius = 1;                    //object space radius of ellipsoid

//------------------------------------------------------------------------------

void stepPhysics(float dt) {
    computeForces();

    //for Explicit Euler
    integrateEuler(dt);

    applyProvotDynamicInverse();
}

//------------------------------------------------------------------------------

void computeForces() {
    size_t i = 0;
    /*std::cout << "points " << total_points << std::endl;
    std::cout << "x " << numX << std::endl;*/
    for (i = 0; i < total_points; i++) {
        F[i] = glm::vec3(0);

        //add gravity force only for non-fixed points
        if (i != 0 && i != numX && i != 440 && i != 420)
            F[i] += gravity;

        //add force due to damping of velocity

        F[i] += DEFAULT_DAMPING * V[i];
    }

    //add spring forces
    for (i = 0; i < springs.size(); i++) {
        glm::vec3 p1 = X[springs[i].p1];
        glm::vec3 p2 = X[springs[i].p2];
        glm::vec3 v1 = V[springs[i].p1];
        glm::vec3 v2 = V[springs[i].p2];
        glm::vec3 deltaP = p1 - p2;
        glm::vec3 deltaV = v1 - v2;
        float dist = glm::length(deltaP);

        float leftTerm = -springs[i].Ks * (dist - springs[i].rest_length);
        float rightTerm = springs[i].Kd * (glm::dot(deltaV, deltaP) / dist);
        glm::vec3 springForce = (leftTerm + rightTerm) * glm::normalize(deltaP);

        if (springs[i].p1 != 0 && springs[i].p1 != numX && springs[i].p1 != 440 && springs[i].p1 != 420)
            F[springs[i].p1] += springForce;
        if (springs[i].p2 != 0 && springs[i].p2 != numX && springs[i].p2 != 440 && springs[i].p2 != 420)
            F[springs[i].p2] -= springForce;
    }
}

//------------------------------------------------------------------------------

void integrateEuler(float deltaTime) {
    float deltaTimeMass = deltaTime / mass;
    size_t i = 0;

    for (i = 0; i < total_points; i++) {
        glm::vec3 oldV = V[i];

        if (i != 0 && i != numX && i != 440 && i != 420) {
            V[i] += (F[i] * deltaTimeMass);
            X[i] += deltaTime * oldV;
        }

        if (X[i].y < 0) {
            X[i].y = 0;
        }
    }
}

//------------------------------------------------------------------------------

void applyProvotDynamicInverse() {

    for (size_t i = 0; i < springs.size(); i++) {
        //check the current lengths of all springs
        glm::vec3 p1 = X[springs[i].p1];
        glm::vec3 p2 = X[springs[i].p2];
        glm::vec3 deltaP = p1 - p2;
        float dist = glm::length(deltaP);
        if (dist > springs[i].rest_length) {
            dist -= (springs[i].rest_length);
            dist /= 2.0f;
            deltaP = glm::normalize(deltaP);
            deltaP *= dist;
            if (springs[i].p1 == 0 || springs[i].p1 == numX || springs[i].p1 == 440 || springs[i].p1 == 420) {
                V[springs[i].p2] += deltaP;
            }
            else if (springs[i].p2 == 0 || springs[i].p2 == numX || springs[i].p2 == 440 || springs[i].p2 == 420) {
                V[springs[i].p1] -= deltaP;
            }
            else {
                V[springs[i].p1] -= deltaP;
                V[springs[i].p2] += deltaP;
            }
        }
    }
}

//------------------------------------------------------------------------------

void addSpring(int a, int b, float ks, float kd, int type) {
    Spring spring;
    spring.p1 = a;
    spring.p2 = b;
    spring.Ks = ks;
    spring.Kd = kd;
    spring.type = type;
    glm::vec3 deltaP = X[a] - X[b];
    spring.rest_length = sqrt(glm::dot(deltaP, deltaP));
    springs.push_back(spring);
}

//------------------------------------------------------------------------------

void drawGrid()
{
    glBegin(GL_LINES);
    glColor3f(0.5f, 0.5f, 0.5f);
    for (int i = -GRID_SIZE; i <= GRID_SIZE; i++)
    {
        glVertex3f((float)i, 0, (float)-GRID_SIZE);
        glVertex3f((float)i, 0, (float)GRID_SIZE);

        glVertex3f((float)-GRID_SIZE, 0, (float)i);
        glVertex3f((float)GRID_SIZE, 0, (float)i);
    }
    glEnd();
}

//------------------------------------------------------------------------------

void initCloth() {
    startTime = (float)glfwGetTime();
    currentTime = startTime;

    // get ticks per second
    QueryPerformanceFrequency(&frequency);

    // start timer
    QueryPerformanceCounter(&t1);

    glEnable(GL_DEPTH_TEST);
    int i = 0, j = 0, count = 0;
    int l1 = 0, l2 = 0;
    float ypos = 7.0f;
    int v = numY + 1;
    int u = numX + 1;

    indices.resize(numX * numY * 2 * 3);
    X.resize(total_points);
    V.resize(total_points);
    F.resize(total_points);

    // fill in X
    for (j = 0; j < v; j++) {
        for (i = 0; i < u; i++) {
            X[count++] = glm::vec3(((float(i) / (u - 1)) * 2 - 1) * halfsize / 5, -0.2f, ((float(j) / (v - 1)) * 2 - 1) * halfsize / 5);
        }
    }

    // fill in V
    memset(&(V[0].x), 0, total_points * sizeof(glm::vec3));

    // fill in indices
    GLushort* id = &indices[0];
    for (i = 0; i < numY; i++) {
        for (j = 0; j < numX; j++) {
            int i0 = i * (numX + 1) + j;
            int i1 = i0 + 1;
            int i2 = i0 + (numX + 1);
            int i3 = i2 + 1;
            if ((j + i) % 2) {
                *id++ = i0; *id++ = i2; *id++ = i1;
                *id++ = i1; *id++ = i2; *id++ = i3;
            }
            else {
                *id++ = i0; *id++ = i2; *id++ = i3;
                *id++ = i0; *id++ = i3; *id++ = i1;
            }
        }
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glPointSize(5);

    //wglSwapIntervalEXT(0);

    // setup springs
    // Horizontal
    for (l1 = 0; l1 < v; l1++)  // v
        for (l2 = 0; l2 < (u - 1); l2++) {
            addSpring((l1 * u) + l2, (l1 * u) + l2 + 1, KsStruct, KdStruct, STRUCTURAL_SPRING);
        }

    // Vertical
    for (l1 = 0; l1 < (u); l1++)
        for (l2 = 0; l2 < (v - 1); l2++) {
            addSpring((l2 * u) + l1, ((l2 + 1) * u) + l1, KsStruct, KdStruct, STRUCTURAL_SPRING);
        }

    // Shearing Springs
    for (l1 = 0; l1 < (v - 1); l1++)
        for (l2 = 0; l2 < (u - 1); l2++) {
            addSpring((l1 * u) + l2, ((l1 + 1) * u) + l2 + 1, KsShear, KdShear, SHEAR_SPRING);
            addSpring(((l1 + 1) * u) + l2, (l1 * u) + l2 + 1, KsShear, KdShear, SHEAR_SPRING);
        }

    // Bend Springs
    for (l1 = 0; l1 < (v); l1++) {
        for (l2 = 0; l2 < (u - 2); l2++) {
            addSpring((l1 * u) + l2, (l1 * u) + l2 + 2, KsBend, KdBend, BEND_SPRING);
        }
        addSpring((l1 * u) + (u - 3), (l1 * u) + (u - 1), KsBend, KdBend, BEND_SPRING);
    }
    for (l1 = 0; l1 < (u); l1++) {
        for (l2 = 0; l2 < (v - 2); l2++) {
            addSpring((l2 * u) + l1, ((l2 + 2) * u) + l1, KsBend, KdBend, BEND_SPRING);
        }
        addSpring(((v - 3) * u) + l1, ((v - 1) * u) + l1, KsBend, KdBend, BEND_SPRING);
    }
}