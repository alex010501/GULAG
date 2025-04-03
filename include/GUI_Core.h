#pragma once

#define GLFW_INCLUDE_NONE

#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(__linux__)
#define GLFW_EXPOSE_NATIVE_X11
#elif defined(__APPLE__)
#define GLFW_EXPOSE_NATIVE_COCOA
#endif


// Systen include
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <sstream>
#include <iomanip>

// GUI include
#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <stb_image.h>

// OSG include
#include <osgViewer/Viewer>
#include <osgViewer/config/SingleWindow>
#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgGA/TrackballManipulator>

// Helper include
#include <sigslot.h>

#include <timer.h>
#include <simState.h>

#include <config.h>

// UI
#include <UI/UIWindow.h>
#include <UI/UIWindow_Console.h>
#include <UI/UIWindow_LibraryViewer.h>
#include <UI/UIWindow_PlotingWorkspace.h>
#include <UI/UIWindow_PropertiesViewer.h>
#include <UI/UIWindow_SceneTreeViewer.h>
#include <UI/UIWindow_ToolPanel.h>
#include <UI/UIWindow_AddEntityDialog.h>

// 3D scene
#include <3D/Scene.h>


// OSG viewer manipulator
class CustomTrackballManipulator : public osgGA::TrackballManipulator
{
public:
    void pan(double dx, double dy, double scale)
    {
        panModel(-dx * scale, dy * scale);
    }

    void rotate(float dx, float dy, double scale)
    {
        rotateTrackball(0.0, 0.0, -dx, dy, scale);
    }
};

// Main window
class CoreWindow: public sigslot::has_slots<>
{
private:
    // OSG stuff
    osgViewer::Viewer m_viewer;
    osg::ref_ptr<CustomTrackballManipulator> m_manipulator;
    
    // GLFW stuff
    GLFWwindow* m_window;
    GLFWimage* m_icon;
    int m_width;
    int m_height;
    const char* m_title;
    const char* m_iconPath;

    // ImGui stuff
    bool imGuiDockspaceInit = false;

    // Simulation stuff
    simState m_simState;
    int m_simFrequency;
    double m_simDuration;
    double m_currentTime;

    // Input
    bool rightMousePressed = false;
    bool leftMousePressed = false;
    bool middleMousePressed = false;
    bool shiftPressed = false;
    double lastX = 0;
    double lastY = 0;

    // Timers
    Timer m_fpsTimer;
    Timer m_simTimer;

    // Signals
    sigslot::signal3<char, std::time_t, const char*> signal_console;
    sigslot::signal2<double, double> signal_play;
    sigslot::signal0<> signal_stop;
    sigslot::signal1<Entity*> signal_selectEntityProperties;

    // Events
    void EventShowEntityWindow(entityType p_entityType);
    void EventAddEntity(EntityInfo p_entityInfo);
    void EventSelectEntity(Entity* p_entity);

    void EventNewFile();
    void EventOpenFile(std::string p_filePath);
    void EventSave();
    void EventSaveAs(std::string p_filePath);

    void EventUndo();
    void EventRedo();

    void EventCut();
    void EventCopy();
    void EventPaste();

    void EventPlay(int p_frequency, float p_duration);
    void EventPause();
    void EventStop();

    void EventOnError(int p_error,std::string p_description);

protected:
    // GUI stuff
    std::vector<UIWindow*>    m_GUIWindows;
    UIWindow_Console          ConsoleWindow;
    UIWindow_LibraryViewer    LibraryWindow;
    UIWindow_PropertiesViewer PropertiesWindow;
    UIWindow_SceneTreeViewer  SceneTreeWindow;
    UIWindow_ToolPanel        ToolPanel;
    UIWindow_PlotingWorkspace PlotingWindow;
    UIWindow_AddEntityDialog  AddEntityDialog;

    // OSG stuff
    osg::Vec4 m_clearColor = osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f);

    // Scene
    Scene m_scene;
    
    // Initialization functions
    void initSignals();
    int initGLFW();
    void imguiInit();
    void initOSG();
    void initGuiDockspace();

    // Scene loading
    void loadScene();

    // Drawing
    void draw();
    void drawGUI();

    // Update
    void update();
    void simulationStep(double p_dt);

    // Termination
    void terminateGLFW();
    void imguiTerminate();

    // Callbacks and checkers
    bool isOpen()
    {
        return !glfwWindowShouldClose(m_window);
    }  

    void setCallbacks();
    void resizeCallback(int p_width, int p_height);
    void posCallback(int p_xpos, int p_ypos);
    void keyCallback(int p_key, int p_scancode, int p_action, int p_mods);
    void scrollCallback(double p_xoffset, double p_yoffset);
    void mouseButtonCallback(int p_button, int p_action, int p_mods);
    void cursorPosCallback(double p_xpos, double p_ypos);

public:
    // Constructor
    CoreWindow(const char* p_title, const char* p_iconPath = nullptr, int p_width = 1280, int p_height = 720);

    // Destructor
    ~CoreWindow();

    // Main loop
    int run(const char* scenePath = nullptr);

    
};