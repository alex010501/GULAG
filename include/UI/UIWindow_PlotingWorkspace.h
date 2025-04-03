#pragma once

#include <UI/UIWindow.h>
#include <UI/UIGraph.h>
#include <cmath>
#include <vector>

class UIWindow_PlotingWorkspace: public UIWindow {
private:
    // Add variables here
    std::vector<Oscilloscope*> *m_plottingOscilloscopes;

    // Add class methods here

public:
    // Add signals here
    sigslot::signal0<> signalAddOscilloscope;

    // Add variables here

    // Add class methods here
    UIWindow_PlotingWorkspace(const char* p_title);

    void draw();
    void init();
};