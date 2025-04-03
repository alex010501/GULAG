#pragma once

#include <UI/UIWindow.h>

#include <simState.h>

class UIWindow_ToolPanel: public UIWindow {

private:
    // Variables
    std::vector<int> m_frequencyItems;
    int m_frequencyIndex;
    float m_duration;

    // Icons
    UIHelper::ImageData NewFileIcon;
    UIHelper::ImageData OpenFileIcon;
    UIHelper::ImageData SaveIcon;
    UIHelper::ImageData SaveAsIcon;
    UIHelper::ImageData UndoIcon;
    UIHelper::ImageData RedoIcon;
    UIHelper::ImageData CutIcon;
    UIHelper::ImageData CopyIcon;
    UIHelper::ImageData PasteIcon;
    UIHelper::ImageData TimerIcon;
    UIHelper::ImageData FrequencyIcon;
    UIHelper::ImageData SpeedIcon;
    UIHelper::ImageData CalculateIcon;
    UIHelper::ImageData StopIcon;
    UIHelper::ImageData Play_PauseIcon;

    // Private methods
    void loadIcons();
    void fileButtons();
    void editButtons();
    void frequency_durationInput();
    void playButtons();

public:
    // Variables
    simState m_state;
    
    // Event senders
    sigslot::signal0<> signal_NewFile;
    sigslot::signal1<std::string> signal_OpenFile;
    sigslot::signal0<> signal_Save;
    sigslot::signal1<std::string> signal_SaveAs;
    sigslot::signal0<> signal_Undo;
    sigslot::signal0<> signal_Redo;
    sigslot::signal0<> signal_Cut;
    sigslot::signal0<> signal_Copy;
    sigslot::signal0<> signal_Paste;
    sigslot::signal2<int, float> signal_Play;
    sigslot::signal0<> signal_Pause;
    sigslot::signal0<> signal_Stop;

    // Add class methods here
    UIWindow_ToolPanel(const char* p_title);
    void draw();
    void init();
};