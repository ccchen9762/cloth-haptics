//==============================================================================
/*
    Yibo Wen
*/
//==============================================================================

//------------------------------------------------------------------------------
#include "chai3d.h"
#include "GEL3D.h"
//------------------------------------------------------------------------------
using namespace chai3d;
using namespace std;
//------------------------------------------------------------------------------
#include <GLFW/glfw3.h>
//#include "GL/glut.h"
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> //for matrices
#include <glm/gtc/type_ptr.hpp>
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// GENERAL SETTINGS
//------------------------------------------------------------------------------

// stereo Mode
/*
    C_STEREO_DISABLED:            Stereo is disabled
    C_STEREO_ACTIVE:              Active stereo for OpenGL NVDIA QUADRO cards
    C_STEREO_PASSIVE_LEFT_RIGHT:  Passive stereo where L/R images are rendered next to each other
    C_STEREO_PASSIVE_TOP_BOTTOM:  Passive stereo where L/R images are rendered above each other
*/
cStereoMode stereoMode = C_STEREO_DISABLED;

// fullscreen mode
bool fullscreen = false;

// mirrored display
bool mirroredDisplay = false;


//------------------------------------------------------------------------------
// DECLARED CHAI3D VARIABLES
//------------------------------------------------------------------------------

// a world that contains all objects of the virtual environment
cWorld* world;

// a camera to render the world in the window display
cCamera* camera;

// a light source to illuminate the objects in the world
cDirectionalLight* light;

// a haptic device handler
cHapticDeviceHandler* handler;

// a pointer to the current haptic device
cGenericHapticDevicePtr hapticDevice;

// force scale factor
double deviceForceScale;

// scale factor between the device workspace and cursor workspace
double workspaceScaleFactor;

// desired workspace radius of the virtual cursor
double cursorWorkspaceRadius;

// a label to display the rate [Hz] at which the simulation is running
cLabel* labelHapticRate;

// a small sphere (cursor) representing the haptic device 
//cShapeSphere* cursor;

// flag to indicate if the haptic simulation currently running
bool simulationRunning = false;

// flag to indicate if the haptic simulation has terminated
bool simulationFinished = false;

// a frequency counter to measure the simulation graphic rate
cFrequencyCounter freqCounterGraphics;

// a frequency counter to measure the simulation haptic rate
cFrequencyCounter freqCounterHaptics;

// haptic thread
cThread* hapticsThread;

// a handle to window display context
GLFWwindow* window = NULL;

// current width of window
int width = 0;

// current height of window
int height = 0;

// swap interval for the display context (vertical synchronization)
int swapInterval = 1;

// information about computer screen and GLUT display window
//int screenW;
//int screenH;
//int windowW;
//int windowH;
//int windowPosX;
//int windowPosY;

//---------------------------------------------------------------------------
// GEL
//---------------------------------------------------------------------------

// deformable world
cGELWorld* defWorld;

// object mesh
cGELMesh* defObject;

// dynamic nodes
cGELSkeletonNode* nodes[10][10];

// haptic device model
cShapeSphere* device;
double deviceRadius;

// radius of the dynamic model sphere (GEM)
double radius;

// stiffness properties between the haptic device tool and the model (GEM)
double stiffness;

//------------------------------------------------------------------------------
// DECLARED GRAPHICS VARIABLES
//------------------------------------------------------------------------------

// grid size
int numX = 20, numY = 20;
const size_t total_points = (numX + 1) * (numY + 1);
float fullsize = 4.0f;
float halfsize = fullsize / 2.0f;

// FPS
float timeStep = 1 / 120.0f;
float currentTime = 0;
double accumulator = timeStep;
int selected_index = -1;
bool bDrawPoints = false;

struct Spring {
    int p1, p2;
    float rest_length;
    float Ks, Kd;
    int type;
};

vector<GLushort> indices;
vector<Spring> springs;

vector<glm::vec3> X;
vector<glm::vec3> V;
vector<glm::vec3> F;

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
// DECLARED CHAI3D FUNCTIONS
//------------------------------------------------------------------------------

// callback when the window display is resized
void resizeWindow(int w, int h);

// callback when a key is pressed
void keySelect(unsigned char key, int x, int y);

// callback to render graphic scene
void updateGraphics(void);

// main haptics simulation loop
void updateHaptics(void);

// callback of GLUT timer
void graphicsTimer(int data);

// function that closes the application
void close(void);

// compute forces between tool and environment
cVector3d computeForce(const cVector3d& a_cursor,
    double a_cursorRadius,
    const cVector3d& a_spherePos,
    double a_radius,
    double a_stiffness);

//------------------------------------------------------------------------------
// DECLARED GRAPHICS FUNCTIONS
//------------------------------------------------------------------------------

// move to next state
void stepPhysics(float dt);

// add spring to the grid
void addSpring(int a, int b, float ks, float kd, int type);

// callback when mouse is down
void onMouseDown(int button, int s, int x, int y);

// callback when mouse is moved
void onMouseMove(int x, int y);

// draw lines of the grid
void drawGrid();

// initialize scene
void initGL();

// update cloth display in updateGraphics
void onRender();

// compute current forces
void computeForces();

// discrete step with euler
void integrateEuler(float deltaTime);

// apply dynamic inverse
void applyProvotDynamicInverse();

// callback when idle
void onIdle();

void errorCallback(int error, const char* a_description);

void keyCallback(GLFWwindow* a_window, int a_key, int a_scancode, int a_action, int a_mods);

void windowSizeCallback(GLFWwindow* a_window, int a_width, int a_height);

void mouseButtonCallback(GLFWwindow* a_window, int a_button, int a_action, int a_mods);


//==============================================================================
/*
    TEMPLATE: main.cpp

    Haptics rendering for deformable objects.
*/
//==============================================================================

int main(int argc, char* argv[])
{
    //--------------------------------------------------------------------------
    // INITIALIZATION
    //--------------------------------------------------------------------------

    cout << endl;
    cout << "-----------------------------------" << endl;
    cout << "CHAI3D" << endl;
    cout << "-----------------------------------" << endl << endl << endl;
    cout << "Keyboard Options:" << endl << endl;
    cout << "[f] - Enable/Disable full screen mode" << endl;
    cout << "[m] - Enable/Disable display points" << endl;
    cout << "[q] - Exit application" << endl;
    cout << endl << endl;


    //--------------------------------------------------------------------------
    // OPENGL - WINDOW DISPLAY
    //--------------------------------------------------------------------------

    // initialize GLFW library
    if (!glfwInit())
    {
        cout << "failed initialization" << endl;
        cSleepMs(1000);
        return 1;
    }

    // set error callback
    glfwSetErrorCallback(errorCallback);

    // compute desired size of window
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    int w = 0.8 * mode->height;
    int h = 0.5 * mode->height;
    int x = 0.5 * (mode->width - w);
    int y = 0.5 * (mode->height - h);

    // set OpenGL version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    // set active stereo mode
    if (stereoMode == C_STEREO_ACTIVE)
    {
        glfwWindowHint(GLFW_STEREO, GL_TRUE);
    }
    else
    {
        glfwWindowHint(GLFW_STEREO, GL_FALSE);
    }

    // create display context
    window = glfwCreateWindow(w, h, "CHAI3D", NULL, NULL);
    if (!window)
    {
        cout << "failed to create window" << endl;
        cSleepMs(1000);
        glfwTerminate();
        return 1;
    }


    // get width and height of window
    glfwGetWindowSize(window, &width, &height);

    // set position of window
    glfwSetWindowPos(window, x, y);

    // set key callback
    glfwSetKeyCallback(window, keyCallback);

    // set resize callback
    glfwSetWindowSizeCallback(window, windowSizeCallback);

    // set current display context
    glfwMakeContextCurrent(window);

    // sets the swap interval for the current display context
    glfwSwapInterval(swapInterval);

    // initialize GLEW library
#ifdef GLEW_VERSION
    if (glewInit() != GLEW_OK)
    {
        cout << "failed to initialize GLEW library" << endl;
        glfwTerminate();
        return 1;
    }
#endif

    //--------------------------------------------------------------------------
    // WORLD - CAMERA - LIGHTING
    //--------------------------------------------------------------------------

    // create a new world.
    world = new cWorld();

    // set the background color of the environment
    world->m_backgroundColor.setBlack();

    // create a camera and insert it into the virtual world
    camera = new cCamera(world);
    world->addChild(camera);

    // position and orient the camera
    camera->set(cVector3d(1.5, 0.0, 1.0),    // camera position (eye)
        cVector3d(0.0, 0.0, 0.0),    // look at position (target)
        cVector3d(0.0, 0.0, 1.0));   // direction of the (up) vector

// set the near and far clipping planes of the camera
    camera->setClippingPlanes(0.01, 100.0);

    // set stereo mode
    camera->setStereoMode(stereoMode);

    // set stereo eye separation and focal length (applies only if stereo is enabled)
    camera->setStereoEyeSeparation(0.01);
    camera->setStereoFocalLength(0.5);

    // set vertical mirrored display mode
    camera->setMirrorVertical(mirroredDisplay);

    // enable multi-pass rendering to handle transparent objects
    camera->setUseMultipassTransparency(true);

    // create a directional light source
    light = new cDirectionalLight(world);

    // insert light source inside world
    world->addChild(light);

    // enable light source
    light->setEnabled(true);

    // define direction of light beam
    light->setDir(-1.0, 0.0, 0.0);

    // create a sphere (cursor) to represent the haptic device
    //device = new cShapeSphere(0.1);

    // insert cursor inside world
    //world->addChild(device);

    // initialize scene
    initGL();


    //--------------------------------------------------------------------------
    // HAPTIC DEVICE
    //--------------------------------------------------------------------------

    // create a haptic device handler
    handler = new cHapticDeviceHandler();

    // get a handle to the first haptic device
    handler->getDevice(hapticDevice, 0);

    // retrieve information about the current haptic device
    cHapticDeviceInfo hapticDeviceInfo = hapticDevice->getSpecifications();

    // open a connection to haptic device
    hapticDevice->open();

    // calibrate device (if necessary)
    hapticDevice->calibrate();

    // retrieve information about the current haptic device
    //cHapticDeviceInfo info = hapticDevice->getSpecifications();

    // display a reference frame if haptic device supports orientations
    if (hapticDeviceInfo.m_sensedRotation == true)
    {
        // display reference frame
        device->setShowFrame(true);

        // set the size of the reference frame
        device->setFrameSize(0.05);
    }

    // if the device has a gripper, enable the gripper to simulate a user switch
    hapticDevice->setEnableGripperUserSwitch(true);

    // desired workspace radius of the cursor
    cursorWorkspaceRadius = 0.2;

    // read the scale factor between the physical workspace of the haptic
    // device and the virtual workspace defined for the tool
    workspaceScaleFactor = cursorWorkspaceRadius / hapticDeviceInfo.m_workspaceRadius;

    // define a scale factor between the force perceived at the cursor and the
    // forces actually sent to the haptic device
    deviceForceScale = 5.0;

    // create a large sphere that represents the haptic device
    deviceRadius = 0.1;
    device = new cShapeSphere(deviceRadius);
    world->addChild(device);
    device->m_material->setWhite();
    device->m_material->setShininess(100);

    // interaction stiffness between tool and deformable model 
    stiffness = 100;

    //-----------------------------------------------------------------------
    // COMPOSE THE VIRTUAL SCENE
    //-----------------------------------------------------------------------

    // create a world which supports deformable object
    defWorld = new cGELWorld();
    world->addChild(defWorld);

    // create a deformable mesh
    defObject = new cGELMesh();
    defWorld->m_gelMeshes.push_front(defObject);

    // set some material color on the object
    cMaterial mat;
    mat.setWhite();
    mat.setShininess(100);
    defObject->setMaterial(mat, true);

    // set object to be transparent
    defObject->setTransparencyLevel(0.65, true, true);

    // build dynamic vertices
    defObject->buildVertices();

    // set default properties for skeleton nodes
    cGELSkeletonNode::s_default_radius = 0.05;  // [m]
    cGELSkeletonNode::s_default_kDampingPos = 2.5;
    cGELSkeletonNode::s_default_kDampingRot = 0.6;
    cGELSkeletonNode::s_default_mass = 0.002; // [kg]
    cGELSkeletonNode::s_default_showFrame = true;
    cGELSkeletonNode::s_default_color.setBlueCornflower();
    cGELSkeletonNode::s_default_useGravity = true;
    cGELSkeletonNode::s_default_gravity.set(0.00, 0.00, -9.81);
    radius = cGELSkeletonNode::s_default_radius;

    // use internal skeleton as deformable model
    defObject->m_useSkeletonModel = true;

    // create an array of nodes
    for (int y = 0; y < 10; y++)
    {
        for (int x = 0; x < 10; x++)
        {
            cGELSkeletonNode* newNode = new cGELSkeletonNode();
            nodes[x][y] = newNode;
            defObject->m_nodes.push_front(newNode);
            newNode->m_pos.set((-0.45 + 0.1 * (double)x), (-0.43 + 0.1 * (double)y), 0.0);
        }
    }

    // set corner nodes as fixed
    nodes[0][0]->m_fixed = true;
    nodes[0][9]->m_fixed = true;
    nodes[9][0]->m_fixed = true;
    nodes[9][9]->m_fixed = true;

    // set default physical properties for links
    cGELSkeletonLink::s_default_kSpringElongation = 25.0;  // [N/m]
    cGELSkeletonLink::s_default_kSpringFlexion = 0.5;   // [Nm/RAD]
    cGELSkeletonLink::s_default_kSpringTorsion = 0.1;   // [Nm/RAD]
    cGELSkeletonLink::s_default_color.setBlueCornflower();

    // create links between nodes
    for (int y = 0; y < 9; y++)
    {
        for (int x = 0; x < 9; x++)
        {
            cGELSkeletonLink* newLinkX0 = new cGELSkeletonLink(nodes[x + 0][y + 0], nodes[x + 1][y + 0]);
            cGELSkeletonLink* newLinkX1 = new cGELSkeletonLink(nodes[x + 0][y + 1], nodes[x + 1][y + 1]);
            cGELSkeletonLink* newLinkY0 = new cGELSkeletonLink(nodes[x + 0][y + 0], nodes[x + 0][y + 1]);
            cGELSkeletonLink* newLinkY1 = new cGELSkeletonLink(nodes[x + 1][y + 0], nodes[x + 1][y + 1]);
            defObject->m_links.push_front(newLinkX0);
            defObject->m_links.push_front(newLinkX1);
            defObject->m_links.push_front(newLinkY0);
            defObject->m_links.push_front(newLinkY1);
        }
    }

    // connect skin (mesh) to skeleton (GEM)
    defObject->connectVerticesToSkeleton(false);

    // show/hide underlying dynamic skeleton model
    defObject->m_showSkeletonModel = false;

    //--------------------------------------------------------------------------
    // WIDGETS
    //--------------------------------------------------------------------------

    // create a font
    cFontPtr font = NEW_CFONTCALIBRI20();

    // create a label to display the haptic rate of the simulation
    labelHapticRate = new cLabel(font);
    labelHapticRate->m_fontColor.setWhite();
    camera->m_frontLayer->addChild(labelHapticRate);


    //--------------------------------------------------------------------------
    // START SIMULATION
    //--------------------------------------------------------------------------

    // create a thread which starts the main haptics rendering loop
    hapticsThread = new cThread();
    hapticsThread->start(updateHaptics, CTHREAD_PRIORITY_HAPTICS);

    // setup callback when application exits
    atexit(close);

    // start the main graphics rendering loop
    windowSizeCallback(window, width, height);

    // main graphic loop
    while (!glfwWindowShouldClose(window))
    {
        // get width and height of window
        glfwGetWindowSize(window, &width, &height);

        // render graphics
        updateGraphics();

        // swap buffers
        glfwSwapBuffers(window);

        // process events
        glfwPollEvents();

        // signal frequency counter
        freqCounterGraphics.signal(1);
    }

    // close window
    glfwDestroyWindow(window);

    // terminate GLFW library
    glfwTerminate();

    // exit
    return (0);
}

//---------------------------------------------------------------------------

void windowSizeCallback(GLFWwindow* a_window, int a_width, int a_height)
{
    // update window size
    width = a_width;
    height = a_height;
}

//------------------------------------------------------------------------------

void errorCallback(int a_error, const char* a_description)
{
    cout << "Error: " << a_description << endl;
}

//------------------------------------------------------------------------------

void keyCallback(GLFWwindow* a_window, int a_key, int a_scancode, int a_action, int a_mods)
{
    // filter calls that only include a key press
    if ((a_action != GLFW_PRESS) && (a_action != GLFW_REPEAT))
    {
        return;
    }

    // option - exit
    else if ((a_key == GLFW_KEY_ESCAPE) || (a_key == GLFW_KEY_Q))
    {
        glfwSetWindowShouldClose(a_window, GLFW_TRUE);
    }

    // option - show/hide skeleton
    else if (a_key == GLFW_KEY_S)
    {
        defObject->m_showSkeletonModel = !defObject->m_showSkeletonModel;
    }

    // option - toggle fullscreen
    else if (a_key == GLFW_KEY_F)
    {
        // toggle state variable
        fullscreen = !fullscreen;

        // get handle to monitor
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();

        // get information about monitor
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        // set fullscreen or window mode
        if (fullscreen)
        {
            glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            glfwSwapInterval(swapInterval);
        }
        else
        {
            int w = 0.8 * mode->height;
            int h = 0.5 * mode->height;
            int x = 0.5 * (mode->width - w);
            int y = 0.5 * (mode->height - h);
            glfwSetWindowMonitor(window, NULL, x, y, w, h, mode->refreshRate);
            glfwSwapInterval(swapInterval);
        }
    }

    // option - toggle vertical mirroring
    else if (a_key == GLFW_KEY_M)
    {
        mirroredDisplay = !mirroredDisplay;
        camera->setMirrorVertical(mirroredDisplay);
    }
    else if (a_key == GLFW_KEY_P)
    {
        bDrawPoints = !bDrawPoints;
    }
    else if (a_key == GLFW_KEY_Q || a_key == GLFW_KEY_ESCAPE)
    {
        close();
        exit(0);
    }
}

//------------------------------------------------------------------------------

void close(void)
{
    // stop the simulation
    simulationRunning = false;

    // wait for graphics and haptics loops to terminate
    while (!simulationFinished) { cSleepMs(100); }

    // close haptic device
    hapticDevice->close();

    // delete resources
    delete hapticsThread;
    delete world;
    delete handler;

    // clear graphics simulation
    X.clear();
    V.clear();
    F.clear();

    indices.clear();
    springs.clear();
}

//------------------------------------------------------------------------------

//void graphicsTimer(int data)
//{
//    if (simulationRunning)
//    {
//        glutPostRedisplay();
//    }
//
//    glutTimerFunc(50, graphicsTimer, 0);
//}

//------------------------------------------------------------------------------

void updateGraphics(void)
{
    /////////////////////////////////////////////////////////////////////
    // UPDATE WIDGETS
    /////////////////////////////////////////////////////////////////////

    // display haptic rate data
    labelHapticRate->setText(cStr(freqCounterGraphics.getFrequency(), 0) + " Hz");

    // update position of label
    labelHapticRate->setLocalPos((int)(0.5 * (width - labelHapticRate->getWidth())), 15);


    // update skins deformable objects
    defWorld->updateSkins(true);

    /////////////////////////////////////////////////////////////////////
    // RENDER SCENE
    /////////////////////////////////////////////////////////////////////

    // update shadow maps (if any)
    world->updateShadowMaps(false, mirroredDisplay);

    // render world
    camera->renderView(width, height);

    // render cloth
    //onRender();
    drawGrid();

    // swap buffers
    //glutSwapBuffers();

    // wait until all GL commands are completed
    glFinish();

    // check for any OpenGL errors
    GLenum err;
    err = glGetError();
    if (err != GL_NO_ERROR) cout << "Error:  %s\n" << gluErrorString(err);
}

//------------------------------------------------------------------------------

void updateHaptics(void)
{
    // initialize precision clock
    cPrecisionClock clock;
    clock.reset();

    // simulation in now running
    simulationRunning = true;
    simulationFinished = false;

    // main haptic simulation loop
    while (simulationRunning)
    {
        // stop clock
        double time = cMin(0.001, clock.stop());

        // restart clock
        clock.start(true);

        // read position from haptic device
        cVector3d pos;
        hapticDevice->getPosition(pos);
        pos.mul(workspaceScaleFactor);
        device->setLocalPos(pos);

        // clear all external forces
        defWorld->clearExternalForces();

        //// compute reaction forces
        cVector3d force(0.0, 0.0, 0.0);
        for (int y = 0; y < 10; y++)
        {
            for (int x = 0; x < 10; x++)
            {
                cVector3d nodePos = nodes[x][y]->m_pos;
                cVector3d f = computeForce(pos, deviceRadius, nodePos, radius, stiffness);
                cVector3d tmpfrc = -1.0 * f;
                nodes[x][y]->setExternalForce(tmpfrc);
                force.add(f);
            }
        }

        //// integrate dynamics
        defWorld->updateDynamics(time);

        //// scale force
        force.mul(deviceForceScale / workspaceScaleFactor);

        //// send forces to haptic device
        hapticDevice->setForce(force);

        // signal frequency counter
        freqCounterHaptics.signal(1);
    }

    // exit haptics thread
    simulationFinished = true;


    // initialize frequency counter
    //frequencyCounter.reset();

    //// simulation in now running
    //simulationRunning = true;
    //simulationFinished = false;

    //// main haptic simulation loop
    //while (simulationRunning)
    //{
    //    /////////////////////////////////////////////////////////////////////
    //    // READ HAPTIC DEVICE
    //    /////////////////////////////////////////////////////////////////////

    //    // read position 
    //    cVector3d position;
    //    hapticDevice->getPosition(position);

    //    // read orientation 
    //    cMatrix3d rotation;
    //    hapticDevice->getRotation(rotation);

    //    // read user-switch status (button 0)
    //    bool button = false;
    //    hapticDevice->getUserSwitch(0, button);


    //    /////////////////////////////////////////////////////////////////////
    //    // UPDATE 3D CURSOR MODEL
    //    /////////////////////////////////////////////////////////////////////

    //    // update position and orienation of cursor
    //    device->setLocalPos(position);
    //    device->setLocalRot(rotation);

    //    /////////////////////////////////////////////////////////////////////
    //    // COMPUTE FORCES
    //    /////////////////////////////////////////////////////////////////////

    //    cVector3d force(0, 0, 0);
    //    cVector3d torque(0, 0, 0);
    //    double gripperForce = 0.0;


    //    /////////////////////////////////////////////////////////////////////
    //    // APPLY FORCES
    //    /////////////////////////////////////////////////////////////////////

    //    // send computed force, torque, and gripper force to haptic device
    //    hapticDevice->setForceAndTorqueAndGripperForce(force, torque, gripperForce);

    //    // update frequency counter
    //    frequencyCounter.signal(1);
    //}

    //// exit haptics thread
    //simulationFinished = true;
}

//------------------------------------------------------------------------------

void mouseButtonCallback(GLFWwindow* a_window, int a_button, int a_action, int a_mods)
{
    double x, y;
    glfwGetCursorPos(window, &x, &y);

    if (a_button == GLFW_MOUSE_BUTTON_LEFT && a_action == GLFW_PRESS)
    {
        oldX = x;
        oldY = y;
        int window_y = (height - y);
        float norm_y = float(window_y) / float(height / 2.0);
        int window_x = x;
        float norm_x = float(window_x) / float(width / 2.0);
        
        float winZ = 0;
        glReadPixels(x, height - y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);
        if (winZ == 1)
            winZ = 0;
        double objX = 0, objY = 0, objZ = 0;
        gluUnProject(window_x, window_y, winZ, MV, P, viewport, &objX, &objY, &objZ);
        glm::vec3 pt(objX, objY, objZ);
        size_t i = 0;
        for (i = 0; i < total_points; i++) {
            if (glm::distance(X[i], pt) < 0.1) {
                selected_index = i;
                printf("Intersected at %d\n", i);
                break;
            }
        }
    }

    if (a_button == GLFW_MOUSE_BUTTON_MIDDLE)
        state = 0;
    else
        state = 1;
        
    if (a_action == GLFW_RELEASE) {
        selected_index = -1;
        glfwSetCursorPos(window, x, y);
    }
}

//void onMouseDown(int button, int s, int x, int y)
//{
//    if (s == GLUT_DOWN)
//    {
//        oldX = x;
//        oldY = y;
//        int window_y = (windowH - y);
//        float norm_y = float(window_y) / float(windowH / 2.0);
//        int window_x = x;
//        float norm_x = float(window_x) / float(windowW / 2.0);
//
//        float winZ = 0;
//        glReadPixels(x, windowH - y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);
//        if (winZ == 1)
//            winZ = 0;
//        double objX = 0, objY = 0, objZ = 0;
//        gluUnProject(window_x, window_y, winZ, MV, P, viewport, &objX, &objY, &objZ);
//        glm::vec3 pt(objX, objY, objZ);
//        size_t i = 0;
//        for (i = 0; i < total_points; i++) {
//            if (glm::distance(X[i], pt) < 0.1) {
//                selected_index = i;
//                printf("Intersected at %d\n", i);
//                break;
//            }
//        }
//    }
//
//    if (button == GLUT_MIDDLE_BUTTON)
//        state = 0;
//    else
//        state = 1;
//
//    if (s == GLUT_UP) {
//        selected_index = -1;
//        glutSetCursor(GLUT_CURSOR_INHERIT);
//    }
//}

//------------------------------------------------------------------------------

//void onMouseMove(int x, int y)
//{
//    if (selected_index == -1) {
//        if (state == 0)
//            dist *= (1 + (y - oldY) / 60.0f);
//        else
//        {
//            rY += (x - oldX) / 5.0f;
//            rX += (y - oldY) / 5.0f;
//        }
//    }
//    else {
//        float delta = 1500 / abs(dist);
//        float valX = (x - oldX) / delta;
//        float valY = (oldY - y) / delta;
//        if (abs(valX) > abs(valY))
//            glutSetCursor(GLUT_CURSOR_LEFT_RIGHT);
//        else
//            glutSetCursor(GLUT_CURSOR_UP_DOWN);
//        V[selected_index] = glm::vec3(0);
//        X[selected_index].x += Right[0] * valX;
//        float newValue = X[selected_index].y + Up[1] * valY;
//        if (newValue > 0)
//            X[selected_index].y = newValue;
//        X[selected_index].z += Right[2] * valX + Up[2] * valY;
//
//        //V[selected_index].x = 0;
//        //V[selected_index].y = 0;
//        //V[selected_index].z = 0;
//    }
//    oldX = x;
//    oldY = y;
//
//    glutPostRedisplay();
//}

//------------------------------------------------------------------------------

/*void onIdle() {
    //Fixed time stepping + rendering at different fps
    if (accumulator >= timeStep)
    {
        stepPhysics(timeStep);
        accumulator -= timeStep;
    }
    glutPostRedisplay();
}*/

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

void initGL() {
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
            //X[count++] = glm::vec3( ((float(i)/(u-1)) *2-1)* halfsize, fullsize+1, ((float(j)/(v-1) )* fullsize));
            X[count++] = glm::vec3(((float(i) / (u - 1)) * 2 - 1) * halfsize, 2.0f, ((float(j) / (v - 1)) * fullsize));
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

////------------------------------------------------------------------------------
//
//void onRender() {
//    size_t i = 0;
//    float newTime = (float)glutGet(GLUT_ELAPSED_TIME);
//    frameTime = newTime - currentTime;
//    currentTime = newTime;
//    //accumulator += frameTime;
//
//    //Using high res. counter
//    QueryPerformanceCounter(&t2);
//    // compute and print the elapsed time in millisec
//    frameTimeQP = (t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
//    t1 = t2;
//    accumulator += frameTimeQP;
//
//    ++totalFrames;
//    if ((newTime - startTime) > 1000)
//    {
//        float elapsedTime = (newTime - startTime);
//        fps = (totalFrames / elapsedTime) * 1000;
//        startTime = newTime;
//        totalFrames = 0;
//    }
//
//    //sprintf_s(info, "FPS: %3.2f, Frame time (GLUT): %3.4f msecs, Frame time (QP): %3.3f", fps, frameTime, frameTimeQP);
//    //glutSetWindowTitle(info);
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//    glLoadIdentity();
//    glTranslatef(0, 0, dist);
//    glRotatef(rX, 1, 0, 0);
//    glRotatef(rY, 0, 1, 0);
//
//    glGetDoublev(GL_MODELVIEW_MATRIX, MV);
//    viewDir.x = (float)-MV[2];
//    viewDir.y = (float)-MV[6];
//    viewDir.z = (float)-MV[10];
//    Right = glm::cross(viewDir, Up);
//
//    //draw grid
//    drawGrid();
//
//    //draw polygons
//    glColor3f(1, 1, 1);
//    glBegin(GL_TRIANGLES);
//    for (i = 0; i < indices.size(); i += 3) {
//        glm::vec3 p1 = X[indices[i]];
//        glm::vec3 p2 = X[indices[i + 1]];
//        glm::vec3 p3 = X[indices[i + 2]];
//        glVertex3f(p1.x, p1.y, p1.z);
//        glVertex3f(p2.x, p2.y, p2.z);
//        glVertex3f(p3.x, p3.y, p3.z);
//    }
//    glEnd();
//
//    //draw points
//    if (bDrawPoints) {
//        glBegin(GL_POINTS);
//        for (i = 0; i < total_points; i++) {
//            glm::vec3 p = X[i];
//            int is = (i == selected_index);
//            glColor3f((float)!is, (float)is, (float)is);
//            glVertex3f(p.x, p.y, p.z);
//        }
//        glEnd();
//    }
//}

cVector3d computeForce(const cVector3d& a_cursor,
    double a_cursorRadius,
    const cVector3d& a_spherePos,
    double a_radius,
    double a_stiffness)
{
    // compute the reaction forces between the tool and the ith sphere.
    cVector3d force;
    force.zero();
    cVector3d vSphereCursor = a_cursor - a_spherePos;

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
    cVector3d forceDirection = cNormalize(vSphereCursor);
    force = cMul(penetrationDistance * a_stiffness, forceDirection);

    // return result
    return (force);
}