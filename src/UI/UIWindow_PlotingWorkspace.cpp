#include <UI/UIWindow_PlotingWorkspace.h>

UIWindow_PlotingWorkspace::UIWindow_PlotingWorkspace(const char* p_title): UIWindow(p_title)
{

}

void UIWindow_PlotingWorkspace::init()
{
    
}

void UIWindow_PlotingWorkspace::draw()
{
    ImGui::Begin(m_title);

    if (ImGui::Button("Add Oscilloscope"))
    {
        this->signalAddOscilloscope();
    }

    /*for (Oscilloscope oscilloscope : this->m_plottingOscilloscope)
    {
        oscilloscope.show();
    }*/

    ImGui::End();
}