//==============================================================================
/*
    Yibo Wen, Ching-Chih Chen
*/
//==============================================================================
// 

//------------------------------------------------------------------------------
#include "cloth.h"
#include <GLFW/glfw3.h>
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
using namespace chai3d;
using namespace std;

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

// state boolean
bool fullscreen = false;
bool mirroredDisplay = false;


//------------------------------------------------------------------------------
// DECLARED CHAI3D VARIABLES
//------------------------------------------------------------------------------

// a world that contains all objects of the virtual environment
cWorld* world;

// a camera to render the world in the window display
cCamera* camera;
cVector3d cameraPos;
cVector3d cameraLookAt;

// a light source to illuminate the objects in the world
cDirectionalLight* light;

// a haptic device handler
cHapticDeviceHandler* handler;

// a pointer to the current haptic device
cGenericHapticDevicePtr hapticDevice;

cToolCursor* tool;

// force scale factor
double deviceForceScale;

// scale factor between the device workspace and cursor workspace
double workspaceScaleFactor;

// desired workspace radius of the virtual cursor
double cursorWorkspaceRadius;

// a label to display the rate [Hz] at which the simulation is running
cLabel* labelHapticRate;

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
int windowWidth = 0;

// current height of window
int windowHeight = 0;

// swap interval for the display context (vertical synchronization)
int swapInterval = 1;

//------------------------------------------------------------------------------
// DECLARED MACROS
//------------------------------------------------------------------------------

// root resource path
string resourceRoot;

// convert to resource path
#define RESOURCE_PATH(p)    (char*)((resourceRoot+string(p)).c_str())

//---------------------------------------------------------------------------
// GEL
//---------------------------------------------------------------------------

// deformable world
cGELWorld* defWorld;

// object mesh
cMesh* tableObject;
cMesh* clothObject;
cGELMesh* defObject;

// dynamic nodes
cGELSkeletonNode* nodes[21][21];

// haptic device model
cShapeSphere* device;
double deviceRadius;

// radius of the dynamic model sphere (GEM)
double modelRadius;

// stiffness properties between the haptic device tool and the model (GEM)
double stiffness;

float tableHeight = -0.5;

//------------------------------------------------------------------------------
// DECLARED GRAPHICS VARIABLES
//------------------------------------------------------------------------------

//// grid size
extern int numX, numY;
extern size_t total_points;

extern int selected_index;
//
extern std::vector<GLushort> indices;
extern std::vector<glm::vec3> X;
//
extern int oldX, oldY;

extern GLint viewport[4];
extern GLdouble MV[16];
extern GLdouble P[16];

//------------------------------------------------------------------------------
// DECLARED CHAI3D FUNCTIONS
//------------------------------------------------------------------------------

// callback when the window display is resized
void windowSizeCallback(GLFWwindow* a_window, int a_width, int a_height);

// callback when an error GLFW occurs
void errorCallback(int error, const char* a_description);

// callback when a key is pressed
void keyCallback(GLFWwindow* a_window, int a_key, int a_scancode, int a_action, int a_mods);

// callback when a mouse button is pressed
void mouseButtonCallback(GLFWwindow* a_window, int a_button, int a_action, int a_mods);

// callback to render graphic scene
void updateGraphics(void);

// main haptics simulation loop
void updateHaptics(void);

void clothTableCollision(void);

// function that closes the application
void close(void);

// compute forces between tool and environment
cVector3d computeForce(const cVector3d& a_cursor,
    double a_cursorRadius,
    const cVector3d& a_spherePos,
    double a_radius,
    double a_stiffness);


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

    std::cout << std::endl;
    std::cout << "-----------------------------------" << std::endl;
    std::cout << "CHAI3D" << std::endl;
    std::cout << "-----------------------------------" << std::endl << std::endl << std::endl;
    std::cout << "Keyboard Options:" << std::endl << std::endl;
    std::cout << "[f] - Enable/Disable full screen mode" << std::endl;
    std::cout << "[m] - Enable/Disable display points" << std::endl;
    std::cout << "[q] - Exit application" << std::endl;
    std::cout << std::endl << std::endl;

    // parse first arg to try and locate resources
    resourceRoot = string(argv[0]).substr(0, string(argv[0]).find_last_of("/\\") + 1);
    std::cout << string(argv[0]) << std::endl;

    //--------------------------------------------------------------------------
    // OPENGL - WINDOW DISPLAY
    //--------------------------------------------------------------------------

    // initialize GLFW library
    if (!glfwInit())
    {
        std::cout << "failed initialization" << std::endl;
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
        std::cout << "failed to create window" << std::endl;
        cSleepMs(1000);
        glfwTerminate();
        return 1;
    }


    // get width and height of window
    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    // set position of window
    glfwSetWindowPos(window, x, y);

    // set key callback
    glfwSetKeyCallback(window, keyCallback);

    // set mouse button callback
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

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

    cameraPos = cVector3d(0.0, 0.8, 1.0);
    cameraLookAt = cVector3d(0.0, 0.0, 0.3);

    // position and orient the camera
    camera->set(cameraPos,    // camera position (eye)
        cameraLookAt,    // look at position (target)
        cVector3d(0.0, 1.0, 0.0));   // direction of the (up) vector

// set the near and far clipping planes of the camera
    camera->setClippingPlanes(0.01, 100.0);

    // set stereo mode
    camera->setStereoMode(stereoMode);

    // set stereo eye separation and focal length (applies only if stereo is enabled)
    camera->setStereoEyeSeparation(0.02);
    camera->setStereoFocalLength(3.0);

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
    light->setDir(0.0, -1.0, -1.0);

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
        device->setShowFrame(false);

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
    
    //-----------------------------------------------------------------------
    // create table mesh
    //-----------------------------------------------------------------------

    float toolRadius = 0.1;
    double maxStiffness = hapticDeviceInfo.m_maxLinearStiffness / workspaceScaleFactor;

    tableObject = new cMesh();

    // create plane

    // create three new vertices
    int vertex0 = tableObject->newVertex();
    int vertex1 = tableObject->newVertex();
    int vertex2 = tableObject->newVertex();
    int vertex3 = tableObject->newVertex();

    // set position of each vertex
    tableObject->m_vertices->setLocalPos(vertex0, -1.0, 0.0, -1.0);
    tableObject->m_vertices->setTexCoord(vertex0, 0.0, 0.0);

    tableObject->m_vertices->setLocalPos(vertex1, 1.0, 0.0, -1.0);
    tableObject->m_vertices->setTexCoord(vertex1, 1.0, 0.0);

    tableObject->m_vertices->setLocalPos(vertex2, 1.0, 0.0, 1.0);
    tableObject->m_vertices->setTexCoord(vertex2, 1.0, 1.0);

    tableObject->m_vertices->setLocalPos(vertex3, -1.0, 0.0, 1.0);
    tableObject->m_vertices->setTexCoord(vertex3, 0.0, 1.0);

    // create new triangle from vertices
    tableObject->newTriangle(vertex0, vertex1, vertex2);
    tableObject->newTriangle(vertex0, vertex2, vertex3);

    // create collision detector
    tableObject->createAABBCollisionDetector(toolRadius);

    world->addChild(tableObject);

    // set the position of the object at the center of the world
    tableObject->setLocalPos(0.0, tableHeight, 0.0);

    // set graphic properties
    bool fileload;
    tableObject->m_texture = cTexture2d::create();
    fileload = tableObject->m_texture->loadFromFile(RESOURCE_PATH("../resources/images/brownboard.jpg"));
    if (!fileload)
    {
#if defined(_MSVC)
        fileload = tableObject->m_texture->loadFromFile("../../../bin/resources/images/brownboard.jpg");
#endif
    }
    if (!fileload)
    {
        cout << "Error - Texture image failed to load correctly." << endl;
        close();
        return (-1);
    }

    // enable texture mapping
    tableObject->setUseTexture(true);
    tableObject->m_material->setWhite();

    // create normal map from texture data
    cNormalMapPtr normalMap = cNormalMap::create();
    normalMap->createMap(tableObject->m_texture);
    tableObject->m_normalMap = normalMap;

    // set haptic properties
    tableObject->m_material->setStiffness(0.3 * maxStiffness);
    tableObject->m_material->setStaticFriction(0.2);
    tableObject->m_material->setDynamicFriction(0.2);
    tableObject->m_material->setTextureLevel(0.2);
    tableObject->m_material->setHapticTriangleSides(true, false);

    //-----------------------------------------------------------------------
    // create untouchable cloth mesh
    //-----------------------------------------------------------------------
    
    clothObject = new cMesh();
    world->addChild(clothObject);

    // set the position of the object at the center of the world
    clothObject->setLocalPos(0.0, 0.0, 0.0);

    // Since we want to see our polygons from both sides, we disable culling.
    clothObject->setUseCulling(false);

    initCloth();

    for (int i = 0; i < indices.size(); i += 3) {
        // define triangle points
        cVector3d p0 = cVector3d(X[indices[i + 0]].x, X[indices[i + 0]].y, X[indices[i + 0]].z);
        cVector3d p1 = cVector3d(X[indices[i + 1]].x, X[indices[i + 1]].y, X[indices[i + 1]].z);
        cVector3d p2 = cVector3d(X[indices[i + 2]].x, X[indices[i + 2]].y, X[indices[i + 2]].z);

        // define a triangle color
        cColorf color;
        color.set((X[indices[i + 0]].x + 1) * 0.5, (X[indices[i + 0]].z + 1) * 0.5, 0.5);

        // create three new vertices
        int vertex0 = clothObject->newVertex();
        int vertex1 = clothObject->newVertex();
        int vertex2 = clothObject->newVertex();

        // set position of each vertex
        clothObject->m_vertices->setLocalPos(vertex0, p0);
        clothObject->m_vertices->setLocalPos(vertex1, p1);
        clothObject->m_vertices->setLocalPos(vertex2, p2);

        // assign color to each vertex
        clothObject->m_vertices->setColor(vertex0, color);
        clothObject->m_vertices->setColor(vertex1, color);
        clothObject->m_vertices->setColor(vertex2, color);

        // create new triangle from vertices
        clothObject->newTriangle(vertex0, vertex1, vertex2);
    }

    // compute surface normals
    clothObject->computeAllNormals();

    // we indicate that we ware rendering triangles by using specific colors for each of them (see above)
    clothObject->setUseVertexColors(true);
    
    //-----------------------------------------------------------------------
    // create a world which supports deformable object &
    // create a deformable mesh
    //-----------------------------------------------------------------------
    
    defWorld = new cGELWorld();
    world->addChild(defWorld);

    defObject = new cGELMesh();
    defWorld->m_gelMeshes.push_front(defObject);

    // build dynamic vertices
    defObject->buildVertices();

    // set default properties for skeleton nodes
    cGELSkeletonNode::s_default_radius = 0.02;  // [m]
    cGELSkeletonNode::s_default_kDampingPos = 2.5;
    cGELSkeletonNode::s_default_kDampingRot = 0.6;
    cGELSkeletonNode::s_default_mass = 0.0002; // [kg]
    cGELSkeletonLink::s_default_color.setBlueAqua();
    cGELSkeletonNode::s_default_showFrame = false;

    cGELSkeletonNode::s_default_useGravity = true;
    cGELSkeletonNode::s_default_gravity.set(0.00, -9.81, 0.00);
    modelRadius = cGELSkeletonNode::s_default_radius;

    // use internal skeleton as deformable model
    defObject->m_useSkeletonModel = true;

    // create an array of nodes
    for (int y = 0; y < 21; y++)
    {
        for (int x = 0; x < 21; x++)
        {
            cGELSkeletonNode* newNode = new cGELSkeletonNode();
            //std::cout << "R: " << newNode->m_color.getR() << std::endl;
            defObject->m_nodes.push_front(newNode);
            newNode->m_pos.set((-0.4 + 0.04 * (double)x), -0.2, (-0.4 + 0.04 * (double)y));
            nodes[x][y] = newNode;
        }
    }

    // set corner nodes as fixed
    nodes[0][0]->m_fixed = true;
    nodes[0][20]->m_fixed = true;
    nodes[20][0]->m_fixed = true;
    nodes[20][20]->m_fixed = true;

    // set default physical properties for links
    cGELSkeletonLink::s_default_kSpringElongation = 25.0;  // [N/m]
    cGELSkeletonLink::s_default_kSpringFlexion = 0.000005;   // [Nm/RAD]
    cGELSkeletonLink::s_default_kSpringTorsion = 0.5;   // [Nm/RAD]

    // create links between nodes
    for (int y = 0; y < 20; y++)
    {
        for (int x = 0; x < 20; x++)
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
    defObject->connectVerticesToSkeleton(true);

    // show/hide underlying dynamic skeleton model
    defObject->m_showSkeletonModel = true;

    //--------------------------------------------------------------------------
    // WIDGETS
    //--------------------------------------------------------------------------

    // create a font
    cFontPtr font = NEW_CFONTCALIBRI20();

    // create a label to display the haptic rate of the simulation
    labelHapticRate = new cLabel(font);
    camera->m_frontLayer->addChild(labelHapticRate);
    labelHapticRate->m_fontColor.setWhite();


    //--------------------------------------------------------------------------
    // START SIMULATION
    //--------------------------------------------------------------------------

    // create a thread which starts the main haptics rendering loop
    hapticsThread = new cThread();
    hapticsThread->start(updateHaptics, CTHREAD_PRIORITY_HAPTICS);

    // setup callback when application exits
    atexit(close);

    // start the main graphics rendering loop
    windowSizeCallback(window, windowWidth, windowHeight);

    // main graphic loop
    while (!glfwWindowShouldClose(window))
    {
        // get width and height of window
        glfwGetWindowSize(window, &windowWidth, &windowHeight);

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
    windowWidth = a_width;
    windowHeight = a_height;
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

    if (a_key == GLFW_KEY_W) {
        cameraPos.z(cameraPos.z() - 0.1);
        cameraLookAt.z(cameraLookAt.z() - 0.1);

        camera->set(cameraPos,    // camera position (eye)
            cameraLookAt,    // look at position (target)
            cVector3d(0.0, 1.0, 0.0));
    }

    if (a_key == GLFW_KEY_A) {
        cameraPos.x(cameraPos.x() - 0.1);
        cameraLookAt.x(cameraLookAt.x() - 0.1);

        camera->set(cameraPos,    // camera position (eye)
            cameraLookAt,    // look at position (target)
            cVector3d(0.0, 1.0, 0.0));
    }

    if (a_key == GLFW_KEY_S) {
        cameraPos.z(cameraPos.z() + 0.1);
        cameraLookAt.z(cameraLookAt.z() + 0.1);

        camera->set(cameraPos,    // camera position (eye)
            cameraLookAt,    // look at position (target)
            cVector3d(0.0, 1.0, 0.0));
    }

    if (a_key == GLFW_KEY_D) {
        cameraPos.x(cameraPos.x() + 0.1);
        cameraLookAt.x(cameraLookAt.x() + 0.1);

        camera->set(cameraPos,    // camera position (eye)
            cameraLookAt,    // look at position (target)
            cVector3d(0.0, 1.0, 0.0));
    }

    // option - exit
    else if ((a_key == GLFW_KEY_ESCAPE) || (a_key == GLFW_KEY_Q))
    {
        glfwSetWindowShouldClose(a_window, GLFW_TRUE);
    }

    // option - show/hide skeleton
    else if (a_key == GLFW_KEY_K)
    {
        defObject->m_showSkeletonModel = !defObject->m_showSkeletonModel;
    }

    else if (a_key == GLFW_KEY_L) {
        bool useWireMode = !clothObject->getWireMode();
        clothObject->setWireMode(useWireMode);
        if (useWireMode)
            cout << "> Wire mode enabled  \r";
        else
            cout << "> Wire mode disabled \r";
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
}

void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {

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
    indices.clear();
}

//------------------------------------------------------------------------------

void updateGraphics(void)
{
    /////////////////////////////////////////////////////////////////////
    // UPDATE WIDGETS
    /////////////////////////////////////////////////////////////////////

    // display haptic rate data
    labelHapticRate->setText(cStr(freqCounterGraphics.getFrequency(), 0) + " Hz / " +
        cStr(freqCounterHaptics.getFrequency(), 0) + " Hz");

    // update position of label
    labelHapticRate->setLocalPos((int)(0.5 * (windowWidth - labelHapticRate->getWidth())), 15);


    // update skins deformable objects
    defWorld->updateSkins(true);

    /////////////////////////////////////////////////////////////////////
    // RENDER SCENE
    /////////////////////////////////////////////////////////////////////

    // update shadow maps (if any)
    world->updateShadowMaps(false, mirroredDisplay);

    // render world
    camera->renderView(windowWidth, windowHeight);

    // render cloth
    //drawGrid();
    for (int i = 0; i < clothObject->getNumVertices(); i+=3) {
        cVector3d p0 = cVector3d(X[indices[i + 0]].x, X[indices[i + 0]].y, X[indices[i + 0]].z);
        cVector3d p1 = cVector3d(X[indices[i + 1]].x, X[indices[i + 1]].y, X[indices[i + 1]].z);
        cVector3d p2 = cVector3d(X[indices[i + 2]].x, X[indices[i + 2]].y, X[indices[i + 2]].z);

        clothObject->m_vertices->setLocalPos(i, p0);
        clothObject->m_vertices->setLocalPos(i+1, p1);
        clothObject->m_vertices->setLocalPos(i+2, p2);
    }

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

        // compute reaction forces
        cVector3d force(0.0, 0.0, 0.0);
        for (int y = 0; y < 21; y++)
        {
            for (int x = 0; x < 21; x++)
            {
                cVector3d nodePos = nodes[x][y]->m_pos;
                cVector3d f = computeForce(pos, deviceRadius, nodePos, modelRadius, stiffness);
                cVector3d tmpfrc = -1.0 * f;

                if (nodePos.get(1) - tableHeight < 0)
                    std::cout << cGELSkeletonLink::s_default_kSpringElongation * (tableHeight - nodePos.get(1)) << std::endl;
                if (nodePos.get(1) - tableHeight < 0) {
                    tmpfrc.y(tmpfrc.get(1) + 
                        cGELSkeletonLink::s_default_kSpringElongation * (tableHeight - nodePos.get(1)));
                }
                nodes[x][y]->setExternalForce(tmpfrc);

                X[y * 21 + x].x = nodePos.x();
                X[y * 21 + x].y = nodePos.y()+0.01;
                X[y * 21 + x].z = nodePos.z();
            }
        }

        // integrate dynamics
        defWorld->updateDynamics(time);

        //// scale force
        force.mul(deviceForceScale / workspaceScaleFactor);

        //// send forces to haptic device
        hapticDevice->setForce(force);

        /* triangle objects */
        // compute global reference frames for each object
        world->computeGlobalPositions(true);

        // update position and orientation of tool
        //tool->updateFromDevice();

        //// compute interaction forces
        //tool->computeInteractionForces();

        //// send forces to haptic device
        //tool->applyToDevice();


        // signal frequency counter
        freqCounterHaptics.signal(1);
    }

    // exit haptics thread
    simulationFinished = true;
}

void clothTableCollision() {
}

//---------------------------------------------------------------------------

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

//------------------------------------------------------------------------------

void mouseButtonCallback(GLFWwindow* a_window, int a_button, int a_action, int a_mods)
{
    double x, y;
    glfwGetCursorPos(window, &x, &y);

    if (a_button == GLFW_MOUSE_BUTTON_LEFT && a_action == GLFW_PRESS)
    {
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
            std::cout << "l press" << std::endl;
        }

        oldX = x;
        oldY = y;
        int window_y = (windowHeight - y);
        float norm_y = float(window_y) / float(windowHeight / 2.0);
        int window_x = x;
        float norm_x = float(window_x) / float(windowWidth / 2.0);
        
        float winZ = 0;
        glReadPixels(x, windowHeight - y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);
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
    if (a_action == GLFW_RELEASE) {
        selected_index = -1;
        glfwSetCursorPos(window, x, y);
    }
}

