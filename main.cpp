//==============================================================================
/*
    Yibo Wen, Ching-Chih Chen
*/
//==============================================================================
// 

//------------------------------------------------------------------------------
#include "Macro.h"
#include "Global.h"
#include "ChaiWorld.h"

#include <GLFW/glfw3.h> // include after chai3d
//------------------------------------------------------------------------------


Rigid* table;
Deformable* cloth;


//------------------------------------------------------------------------------
// TEXT VARIABLES
//------------------------------------------------------------------------------

// a label to display the rate [Hz] at which the simulation is running
chai3d::cLabel* labelHapticRate;

// flag to indicate if the haptic simulation currently running
bool simulationRunning = false;

// flag to indicate if the haptic simulation has terminated
bool simulationFinished = false;

// a frequency counter to measure the simulation graphic rate
chai3d::cFrequencyCounter freqCounterGraphics;

// a frequency counter to measure the simulation haptic rate
chai3d::cFrequencyCounter freqCounterHaptics;

// haptic thread
chai3d::cThread* hapticsThread;

// a handle to window display context
GLFWwindow* window = NULL;

// current width of window
int windowWidth = 0;

// current height of window
int windowHeight = 0;

// swap interval for the display context (vertical synchronization)
int swapInterval = 1;

//------------------------------------------------------------------------------
// STATES
//------------------------------------------------------------------------------

bool isWPressing = false;
bool isAPressing = false;
bool isSPressing = false;
bool isDPressing = false;

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
//void mouseButtonCallback(GLFWwindow* a_window, int a_button, int a_action, int a_mods);

// callback to render graphic scene
void updateGraphics(void);

// main haptics simulation loop
void updateHaptics(void);

//void clothTableCollision(void);

// function that closes the application
void close(void);


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
    resourceRoot = std::string(argv[0]).substr(0, std::string(argv[0]).find_last_of("/\\") + 1);
    std::cout << std::string(argv[0]) << std::endl;

    //--------------------------------------------------------------------------
    // OPENGL - WINDOW DISPLAY
    //--------------------------------------------------------------------------

    // initialize GLFW library
    if (!glfwInit())
    {
        std::cout << "failed initialization" << std::endl;
        chai3d::cSleepMs(1000);
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
    if (kStereoMode == chai3d::C_STEREO_ACTIVE)
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
        chai3d::cSleepMs(1000);
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
    //glfwSetMouseButtonCallback(window, mouseButtonCallback);

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
        std::cout << "failed to initialize GLEW library" << std::endl;
        glfwTerminate();
        return 1;
    }
#endif

    //-----------------------------------------------------------------------
    // COMPOSE THE VIRTUAL SCENE
    //-----------------------------------------------------------------------
    table = new Rigid(4.0, 4.0, chai3d::cVector3d(0.0, 0.0, -0.8), 0.8, 0.3, 0.2, 1.0);
    cloth = new Deformable(21, 21, chai3d::cVector3d(0.0, 0.0, -0.4));

    ChaiWorld::chaiWorld.attachRigidObject(*table);
    ChaiWorld::chaiWorld.attachDeformableObject(*cloth);

    //--------------------------------------------------------------------------
    // WIDGETS
    //--------------------------------------------------------------------------

    // create a font
    chai3d::cFontPtr font = chai3d::NEW_CFONTCALIBRI20();

    // create a label to display the haptic rate of the simulation
    labelHapticRate = new chai3d::cLabel(font);
    ChaiWorld::chaiWorld.getCamera()->m_frontLayer->addChild(labelHapticRate);
    labelHapticRate->m_fontColor.setWhite();

    //--------------------------------------------------------------------------
    // START SIMULATION
    //--------------------------------------------------------------------------

    // create a thread which starts the main haptics rendering loop
    hapticsThread = new chai3d::cThread();
    hapticsThread->start(updateHaptics, chai3d::CTHREAD_PRIORITY_HAPTICS);

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
    std::cout << "Error: " << a_description << std::endl;
}

//------------------------------------------------------------------------------

void keyCallback(GLFWwindow* a_window, int a_key, int a_scancode, int a_action, int a_mods)
{
    switch (a_key)
    {
    case GLFW_KEY_W:
        if (a_action == GLFW_PRESS)
            isWPressing = true;
        else if (a_action == GLFW_RELEASE)
            isWPressing = false;
        break;
    case GLFW_KEY_A:
        if (a_action == GLFW_PRESS)
            isAPressing = true;
        else if (a_action == GLFW_RELEASE)
            isAPressing = false;
        break;
    case GLFW_KEY_S:
        if (a_action == GLFW_PRESS)
            isSPressing = true;
        else if (a_action == GLFW_RELEASE)
            isSPressing = false;
        break;
    case GLFW_KEY_D:
        if (a_action == GLFW_PRESS)
            isDPressing = true;
        else if (a_action == GLFW_RELEASE)
            isDPressing = false;
        break;
    case GLFW_KEY_ESCAPE:
    case GLFW_KEY_Q:
        glfwSetWindowShouldClose(a_window, GLFW_TRUE);
        break;
    case GLFW_KEY_K:
        if (a_action == GLFW_PRESS)
            cloth->getDefObject()->m_showSkeletonModel = !cloth->getDefObject()->m_showSkeletonModel;
        break;
    case GLFW_KEY_L:
        /*bool useWireMode = !clothObject->getWireMode();
        clothObject->setWireMode(useWireMode);
        if (useWireMode)
            std::cout << "> Wire mode enabled  \r";
        else
            std::cout << "> Wire mode disabled \r";*/
        break;
    case GLFW_KEY_F:
        {
            // toggle state variable
            kFullscreen = !kFullscreen;

            // get handle to monitor
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();

            // get information about monitor
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);

            // set fullscreen or window mode
            if (kFullscreen)
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
        break;
    case GLFW_KEY_M:
        kMirroredDisplay = !kMirroredDisplay;
        ChaiWorld::chaiWorld.getCamera()->setMirrorVertical(kMirroredDisplay);
        break;
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
    while (!simulationFinished) { chai3d::cSleepMs(100); }

    // close haptic device
    ChaiWorld::chaiWorld.getHapticDevice()->close();

    // delete resources
    delete hapticsThread;
    delete ChaiWorld::chaiWorld.getWorld();
    delete ChaiWorld::chaiWorld.getHandler();

    // clear graphics simulation
    //X.clear();
    //indices.clear();
}

//------------------------------------------------------------------------------

void updateGraphics(void)
{
    /////////////////////////////////////////////////////////////////////
    // UPDATE WIDGETS
    /////////////////////////////////////////////////////////////////////

    // display haptic rate data
    labelHapticRate->setText(chai3d::cStr(freqCounterGraphics.getFrequency(), 0) + " Hz / " +
        chai3d::cStr(freqCounterHaptics.getFrequency(), 0) + " Hz");

    // update position of label
    labelHapticRate->setLocalPos((int)(0.5 * (windowWidth - labelHapticRate->getWidth())), 15);


    // update skins deformable objects
    ChaiWorld::chaiWorld.getDefWorld()->updateSkins(true);

    /////////////////////////////////////////////////////////////////////
    // RENDER SCENE
    /////////////////////////////////////////////////////////////////////

    // update shadow maps (if any)
    ChaiWorld::chaiWorld.getWorld()->updateShadowMaps(false, kMirroredDisplay);

    // render world
    ChaiWorld::chaiWorld.getCamera()->renderView(windowWidth, windowHeight);

    // render cloth
    /*for (int i = 0; i < clothObject->getNumVertices(); i+=3) {
        chai3d::cVector3d p0 = chai3d::cVector3d(X[indices[i + 0]].x, X[indices[i + 0]].y, X[indices[i + 0]].z);
        chai3d::cVector3d p1 = chai3d::cVector3d(X[indices[i + 1]].x, X[indices[i + 1]].y, X[indices[i + 1]].z);
        chai3d::cVector3d p2 = chai3d::cVector3d(X[indices[i + 2]].x, X[indices[i + 2]].y, X[indices[i + 2]].z);

        clothObject->m_vertices->setLocalPos(i, p0);
        clothObject->m_vertices->setLocalPos(i+1, p1);
        clothObject->m_vertices->setLocalPos(i+2, p2);
    }*/

    // wait until all GL commands are completed
    glFinish();

    // check for any OpenGL errors
    GLenum err;
    err = glGetError();
    if (err != GL_NO_ERROR) std::cout << "Error:  %s\n" << gluErrorString(err);
}

//------------------------------------------------------------------------------

void updateHaptics(void)
{
    // initialize precision clock
    chai3d::cPrecisionClock clock;
    clock.reset();

    // simulation in now running
    simulationRunning = true;
    simulationFinished = false;

    // main haptic simulation loop
    while (simulationRunning)
    {
        // stop clock
        double time = chai3d::cMin(0.001, clock.stop());

        // restart clock
        clock.start(true);

        ChaiWorld::chaiWorld.updateHaptics(time, cloth, table);

        // signal frequency counter
        freqCounterHaptics.signal(1);
    }

    // exit haptics thread
    simulationFinished = true;
}