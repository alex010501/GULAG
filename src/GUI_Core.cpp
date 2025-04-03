#include <GUI_Core.h>

CoreWindow::CoreWindow(const char *p_title,
                       const char* p_iconPath,
                       int p_width, int p_height):
                       m_width(p_width), m_height(p_height), m_title(p_title), m_iconPath(p_iconPath),
                       ConsoleWindow("Console"),
                       LibraryWindow("Library"),
                       PropertiesWindow("Properties"),
                       SceneTreeWindow("Scene Tree"),
                       ToolPanel("Tools"),
                       PlotingWindow("Plots"),
                       AddEntityDialog("Add...")
{
    this->m_clearColor = osg::Vec4(0.8, 0.8, 1.0, 1.0);
    this->m_GUIWindows = {&this->ConsoleWindow,
                          &this->LibraryWindow,
                          &this->PropertiesWindow,
                          &this->SceneTreeWindow,
                          &this->ToolPanel,
                          &this->PlotingWindow,
                          &this->AddEntityDialog};
    this->m_simState = SIM_STOP;
    this->m_simFrequency = 25;
}

CoreWindow::~CoreWindow()
{

}

int CoreWindow::run(const char* scenePath)
{
    // Connect app with GUI windows
    this->initSignals();

    // Initialise GLFW
    int lv_glfwStatus = this->initGLFW();
    if (lv_glfwStatus != 0)
    {
        std::cout << "GLFW Error: " << lv_glfwStatus << std::endl;
        return lv_glfwStatus;
    }

    // Set up GLFW callbacks
    this->setCallbacks();

    // Set up OSG
    this->initOSG();
    
    // Load scene
    this->loadScene();    

    // Setup Dear ImGui context
    this->imguiInit();

    // Create simulation thread
    auto updateMethod = std::bind(&CoreWindow::update, this);
    std::thread updateThread(updateMethod);
    this->m_fpsTimer.setFrequency(60);
    this->m_fpsTimer.start();

    // Start main loop
    while (this->isOpen())
    {        
        this->draw();
        glfwSwapBuffers(this->m_window);
        glfwPollEvents();
        this->m_fpsTimer.wait();
    }

    // Wait for simulation thread
    updateThread.join();

    // ImGui shutdown
    this->imguiTerminate();

    // GLFW shutdown
    this->terminateGLFW();

    return 0;
}

void CoreWindow::initSignals()
{
    // Connect from main window to all GUI windows
    this->signal_console.connect(&(this->ConsoleWindow),   &UIWindow_Console::consoleCallback);
    this->signal_selectEntityProperties.connect(&(this->PropertiesWindow), &UIWindow_PropertiesViewer::setSelectedEntity);

    // Connect from tool panel to main window
    this->ToolPanel.signal_NewFile .connect(this, &CoreWindow::EventNewFile);
    this->ToolPanel.signal_OpenFile.connect(this, &CoreWindow::EventOpenFile);
    this->ToolPanel.signal_Save    .connect(this, &CoreWindow::EventSave);
    this->ToolPanel.signal_SaveAs  .connect(this, &CoreWindow::EventSaveAs);
    this->ToolPanel.signal_Undo    .connect(this, &CoreWindow::EventUndo);
    this->ToolPanel.signal_Redo    .connect(this, &CoreWindow::EventRedo);
    this->ToolPanel.signal_Cut     .connect(this, &CoreWindow::EventCut);
    this->ToolPanel.signal_Copy    .connect(this, &CoreWindow::EventCopy);
    this->ToolPanel.signal_Paste   .connect(this, &CoreWindow::EventPaste);
    this->ToolPanel.signal_Play    .connect(this, &CoreWindow::EventPlay);
    this->ToolPanel.signal_Pause   .connect(this, &CoreWindow::EventPause);
    this->ToolPanel.signal_Stop    .connect(this, &CoreWindow::EventStop);

    this->SceneTreeWindow.signal_showEntityWindow.connect(this, &CoreWindow::EventShowEntityWindow);
    this->SceneTreeWindow.signal_selectEntity.connect(this, &CoreWindow::EventSelectEntity);

    this->AddEntityDialog.signal_CreateEntity.connect(this, &CoreWindow::EventAddEntity);
}

int CoreWindow::initGLFW()
{
    if (!glfwInit())
    {
        // Handle initialization error
        return 1;
    }
    glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_TRUE);   
    // Create GLFW window
    this->m_window = glfwCreateWindow(this->m_width, this->m_height, this->m_title, NULL, NULL);
    if (!this->m_window)
    {
        glfwTerminate();
        return 2;
    }    
    glfwSetWindowSizeLimits(this->m_window, 640, 360, GLFW_DONT_CARE, GLFW_DONT_CARE);
    // Set Icon
    if (this->m_iconPath)
    {
        int lv_image_width, lv_image_height, lv_image_channels;
        unsigned char* lv_image_data = stbi_load(this->m_iconPath, &lv_image_width, &lv_image_height, &lv_image_channels, 0);
        if (lv_image_data)
        {
            this->m_icon = new GLFWimage;
            this->m_icon[0].width = lv_image_width;
            this->m_icon[0].height = lv_image_height;
            this->m_icon[0].pixels = lv_image_data;
            glfwSetWindowIcon(this->m_window, 1, this->m_icon);
        }
        else
        {
            std::cout << "Failed to load image: " << this->m_iconPath << std::endl;
            stbi_image_free(lv_image_data);
            this->m_icon = NULL;
            glfwSetWindowIcon(this->m_window, 0, NULL);
        }
    }
    else
    {
        this->m_icon = NULL;
        glfwSetWindowIcon(this->m_window, 0, NULL);
    }
    glfwMakeContextCurrent(this->m_window);
    glfwSwapInterval(1); // Enable vsync

    // Maximize window
    glfwMaximizeWindow(this->m_window);

    return 0;
}

void CoreWindow::imguiInit()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
    std::string lv_font = "resources/Fonts/";
    lv_font += FONT_NAME;
    io.Fonts->AddFontFromFileTTF(lv_font.c_str(), 18.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());

    ImFontConfig config;
    config.MergeMode = true;
    config.GlyphMinAdvanceX = 18.0f; // Use if you want to make the icon monospaced
    static const ImWchar icon_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    std::string lv_icon_font = "resources/Fonts/FA6_Free-Solid.otf";
    io.Fonts->AddFontFromFileTTF(lv_icon_font.c_str(), 18.0f, &config, icon_ranges);

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(this->m_window, true);
    ImGui_ImplOpenGL3_Init("#version 460 core");

    for (auto window : this->m_GUIWindows)
    {
        window->init();
    }

    this->SceneTreeWindow.setSceneRoot(this->m_scene.getSceneRoot());
}

void CoreWindow::initOSG()
{
    // Initialize OSG viewer
    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->readDISPLAY();
    traits->setUndefinedScreenDetailsToDefaultScreen();
    traits->windowName = this->m_title;    
    this->m_viewer.setUpViewerAsEmbeddedInWindow(0, 0, this->m_width, this->m_height);   
    this->m_viewer.getCamera()->setGraphicsContext(new osgViewer::GraphicsWindowEmbedded(traits.get()));
    const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    this->m_viewer.getCamera()->setViewport(new osg::Viewport(0, 0, mode->width, mode->height));

    // set up viewer
    this->m_viewer.getCamera()->setClearColor(this->m_clearColor);    
    
    // set up manipulator
    this->m_manipulator = new CustomTrackballManipulator;
    osg::Vec3 eye(5.0, 5.0, 5.0);
    osg::Vec3 center(0.0, 0.0, 0.0);
    osg::Vec3 up(0.0, 0.0, 1.0);
    this->m_manipulator->setHomePosition(eye, center, up);
    this->m_viewer.setCameraManipulator(this->m_manipulator);
}

void CoreWindow::loadScene()
{
    this->m_viewer.setSceneData(this->m_scene.getOSGRoot());
}

void CoreWindow::draw()
{
    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    // Dockspace settings
    this->initGuiDockspace();

    // Render subwindows
    this->drawGUI();

    // Prepare ImGui data for rendering
    ImGui::Render();
    ImGui::UpdatePlatformWindows();

    // OSG render
    this->m_viewer.frame();

    // ImGui-OpenGL render
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void CoreWindow::drawGUI()
{
    for (auto window : this->m_GUIWindows)
    {
        window->draw();
    }
}

void CoreWindow::initGuiDockspace()
{
    ImGuiID dockspace_id = ImGui::DockSpaceOverViewport(0U, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
    ImGuiID dock_id_up, dock_id_rightUp, dock_id_rightBottom, dock_id_left, dock_id_center, dock_id_bottom;
    if (!(this->imGuiDockspaceInit))
    {
        this->imGuiDockspaceInit = true;
        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

        ImGui::DockBuilderSplitNode(  dockspace_id,  ImGuiDir_Up,    0.05f,         &dock_id_up, &dock_id_center);
        ImGui::DockBuilderSplitNode(dock_id_center,  ImGuiDir_Left,  0.25f,       &dock_id_left, &dock_id_center);
        ImGui::DockBuilderSplitNode(dock_id_center,  ImGuiDir_Right, 0.3f,     &dock_id_rightUp, &dock_id_center);
        ImGui::DockBuilderSplitNode(dock_id_center,  ImGuiDir_Down,  0.2f,      &dock_id_bottom, &dock_id_center);
        ImGui::DockBuilderSplitNode(dock_id_rightUp, ImGuiDir_Down,  0.6f, &dock_id_rightBottom, &dock_id_rightUp);

        ImGui::DockBuilderDockWindow(     "Tools", dock_id_up);
        ImGui::DockBuilderDockWindow("Scene Tree", dock_id_rightUp);
        ImGui::DockBuilderDockWindow("Properties", dock_id_rightBottom);
        ImGui::DockBuilderDockWindow(     "Plots", dock_id_left);
        ImGui::DockBuilderDockWindow(   "Console", dock_id_bottom);        
        ImGui::DockBuilderDockWindow(   "Library", dock_id_bottom);

        ImGui::DockBuilderFinish(dockspace_id);
    }
}

void CoreWindow::update()
{
    while(this->isOpen())
    {
        if (this->m_simState == SIM_PLAY)
        {
            this->m_simTimer.start();
            this->simulationStep(1.0/this->m_simFrequency);
            this->m_currentTime += 1.0/this->m_simFrequency;
            if (this-> m_currentTime >= this->m_simDuration)
                this->EventStop();
            this->m_simTimer.wait();       
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }       
    }
}

void CoreWindow::simulationStep(double p_dt)
{

}

void CoreWindow::imguiTerminate()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    ImPlot::DestroyContext();
}

void CoreWindow::terminateGLFW()
{
    stbi_image_free(this->m_icon[0].pixels);
    glfwDestroyWindow(this->m_window);
    glfwTerminate();
}

void CoreWindow::resizeCallback(int p_width, int p_height)
{
    this->m_width = p_width;
    this->m_height = p_height;
    this->draw();
}

void CoreWindow::posCallback(int p_x, int p_y)
{
    glfwSetWindowPos(this->m_window, p_x, p_y);
    this->draw();
}

void CoreWindow::keyCallback(int p_key, int p_scancode, int p_action, int p_mods)
{
    if      (p_key == GLFW_KEY_LEFT_SHIFT && p_action == GLFW_PRESS)
    {
        this->shiftPressed = true;
    }
    else if (p_key == GLFW_KEY_LEFT_SHIFT && p_action == GLFW_RELEASE)
    {
        this->shiftPressed = false;
    }
}

void CoreWindow::scrollCallback(double p_xoffset, double p_yoffset)
{
    int k;
    if (this->shiftPressed)
    {
        k = 10;
    }
    else
    {
        k = 1;
    }
    this->m_manipulator->setDistance(this->m_manipulator->getDistance() - k * p_yoffset);
}

void CoreWindow::mouseButtonCallback(int p_button, int p_action, int p_mods)
{
    if      (p_button == GLFW_MOUSE_BUTTON_RIGHT  && p_action == GLFW_PRESS)
    {
        this->rightMousePressed = true;
    }
    else if (p_button == GLFW_MOUSE_BUTTON_RIGHT  && p_action == GLFW_RELEASE)
    {
        this->rightMousePressed = false;
    }
    else if (p_button == GLFW_MOUSE_BUTTON_MIDDLE && p_action == GLFW_PRESS)
    {
        this->middleMousePressed = true;
    }
    else if (p_button == GLFW_MOUSE_BUTTON_MIDDLE && p_action == GLFW_RELEASE)
    {
        this->middleMousePressed = false;
    }
    else if (p_button == GLFW_MOUSE_BUTTON_LEFT   && p_action == GLFW_PRESS)
    {
        this->leftMousePressed = true;
    }
    else if (p_button == GLFW_MOUSE_BUTTON_LEFT   && p_action == GLFW_RELEASE)
    {
        this->leftMousePressed = false;
    }
}

void CoreWindow::cursorPosCallback(double p_x, double p_y)
{
    double dx = p_x - this->lastX;
    double dy = p_y - this->lastY;
    double k1 = 0.001;
    double k2 = 0.0005;
    double l = this->m_manipulator->getDistance();
    if (this->rightMousePressed)
    {
        this->m_manipulator->rotate(dx, dy, k1 * l);
    }
    if (this->middleMousePressed)
    {
        this->m_manipulator->pan(dx, dy, l * k2);
    }
    this->lastX = p_x;
    this->lastY = p_y;
}

void CoreWindow::setCallbacks()
{
    // Set GLFW window user pointer
    glfwSetWindowUserPointer(this->m_window, this);
    
    // Set GLFW callbacks
    auto CoreWindowResizeCallback = [](GLFWwindow* p_window, int p_width, int p_height)
    {
        static_cast<CoreWindow*>(glfwGetWindowUserPointer(p_window))->resizeCallback(p_width, p_height);
    };
    glfwSetWindowSizeCallback(this->m_window, CoreWindowResizeCallback);

    auto CoreWindowPosCallback = [](GLFWwindow* p_window, int p_x, int p_y)
    {
        static_cast<CoreWindow*>(glfwGetWindowUserPointer(p_window))->posCallback(p_x, p_y);
    };
    glfwSetWindowPosCallback(this->m_window, CoreWindowPosCallback);

    auto CoreWindowScrollCallback = [](GLFWwindow* p_window, double p_xoffset, double p_yoffset)
    {
        static_cast<CoreWindow*>(glfwGetWindowUserPointer(p_window))->scrollCallback(p_xoffset, p_yoffset);
    };
    glfwSetScrollCallback(this->m_window, CoreWindowScrollCallback);

    auto CoreWindowMouseButtonCallback = [](GLFWwindow* p_window, int p_button, int p_action, int p_mods)
    {
        static_cast<CoreWindow*>(glfwGetWindowUserPointer(p_window))->mouseButtonCallback(p_button, p_action, p_mods);
    };
    glfwSetMouseButtonCallback(this->m_window, CoreWindowMouseButtonCallback);

    auto CoreWindowCursorPosCallback = [](GLFWwindow* p_window, double p_xpos, double p_ypos)
    {
        static_cast<CoreWindow*>(glfwGetWindowUserPointer(p_window))->cursorPosCallback(p_xpos, p_ypos);
    };
    glfwSetCursorPosCallback(this->m_window, CoreWindowCursorPosCallback);

    auto CoreWindowKeyCallback = [](GLFWwindow* p_window, int p_key, int p_scancode, int p_action, int p_mods)
    {
        static_cast<CoreWindow*>(glfwGetWindowUserPointer(p_window))->keyCallback(p_key, p_scancode, p_action, p_mods);
    };
    glfwSetKeyCallback(this->m_window, CoreWindowKeyCallback);
}

void CoreWindow::EventShowEntityWindow(entityType p_entityType)
{
    this->AddEntityDialog.setEntityType(p_entityType);
    this->AddEntityDialog.show();
}

void CoreWindow::EventAddEntity(EntityInfo p_entityInfo)
{
    p_entityInfo.parent = this->m_scene.getSceneRoot();
    this->m_scene.addEntity(p_entityInfo);
    std::string lv_answer;
    switch (p_entityInfo.type)
    {
    case ENTITY:
        lv_answer = "Added entity - ";
        break;
    case DUMMY:
        lv_answer = "Added dummy - ";
        break;
    case FOLDER:
        lv_answer = "Added folder - ";
        break;
    case OBJECT:
        lv_answer = "Added object - ";
        break;
    case CUBE:
        lv_answer = "Added cube - ";
        break;
    case SPHERE:
        lv_answer = "Added sphere - ";
        break;
    case CYLINDER:
        lv_answer = "Added cylinder - ";
        break;
    case MULTIBODY:
        lv_answer = "Added multibody - ";
        break;   
    default:
        lv_answer = "Added entity - ";
        break;
    }
    lv_answer += p_entityInfo.name;
    this->signal_console('w', std::time(nullptr), lv_answer.c_str());
}

void CoreWindow::EventSelectEntity(Entity* p_entity)
{
    this->m_scene.selectEntity(p_entity);
    this->signal_selectEntityProperties(p_entity);
}

void CoreWindow::EventNewFile()
{
    this->signal_console('w', std::time(nullptr), "Created new file");
}

void CoreWindow::EventOpenFile(std::string p_filePath)
{
    std::string lv_answer = "Opened file ";
    lv_answer += p_filePath;
    this->signal_console('g', std::time(nullptr), lv_answer.c_str());
}

void CoreWindow::EventSave()
{
    this->signal_console('g', std::time(nullptr), "Saved file");
}

void CoreWindow::EventSaveAs(std::string p_filePath)
{
    std::string lv_answer = "File saved at ";
    lv_answer += p_filePath;
    this->signal_console('g', std::time(nullptr), lv_answer.c_str());
}

void CoreWindow::EventUndo()
{
    this->signal_console('w', std::time(nullptr), "Undid action");
}

void CoreWindow::EventRedo()
{
    this->signal_console('w', std::time(nullptr), "Redid action");
}

void CoreWindow::EventCut()
{
    this->signal_console('w', std::time(nullptr), "Cut selection");
}

void CoreWindow::EventCopy()
{
    this->signal_console('w', std::time(nullptr), "Copied selection");
}

void CoreWindow::EventPaste()
{
    this->signal_console('w', std::time(nullptr), "Pasted selection");
}

void CoreWindow::EventPlay(int p_frequency, float p_duration)
{
    if (this->m_simState == SIM_PAUSE)
        this->signal_console('w', std::time(nullptr), "Resume simulation");
    else
    {
        this->m_simFrequency = p_frequency;
        this->m_simTimer.setFrequency(this->m_simFrequency);
        this->m_simDuration = p_duration;

        this->signal_play( 1.0/this->m_simFrequency, p_duration);

        std::stringstream lv_stream;
        lv_stream << std::fixed << std::setprecision(1) << p_duration;

        std::string str = "Simulation started; Selected Frequency: " + std::to_string(p_frequency) + " Hz; Duration: " + lv_stream.str() + " s";
        this->signal_console('w', std::time(nullptr), str.c_str());
    }
    this->m_simState = SIM_PLAY;    
}

void CoreWindow::EventPause()
{
    this->m_simState = SIM_PAUSE;
    this->signal_console('w', std::time(nullptr), "Simulation paused");
}

void CoreWindow::EventStop()
{
    if (this->m_simState == SIM_STOP)
        return;    
    this->m_simState = SIM_STOP;
    this->ToolPanel.m_state = SIM_STOP;
    this->m_currentTime = 0.0;
    this->signal_stop();
    this->signal_console('w', std::time(nullptr), "Simulation stopped");
}

void CoreWindow::EventOnError(int p_error, std::string p_description)
{
    std::string lv_error_str = "Error #" + std::to_string(p_error) + ": " + p_description;
    this->signal_console('r', std::time(nullptr), lv_error_str.c_str());
}