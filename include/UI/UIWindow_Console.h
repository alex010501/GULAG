#pragma once

#include <UI/UIWindow.h>

#include <vector>
#include <ctime>

struct consoleMessage
{
    char color;
    char message[256];
};

class UIWindow_Console: public UIWindow {
private:
    // Add variables here
    char m_commandBuffer[256];
    std::vector<consoleMessage> m_consoleLines;

    // Private methods
    void comandCallback();
    void drawConsoleItem(char p_color, const char* p_message);
public:
    // Add variables here


    // Add class methods here
    UIWindow_Console(const char* p_title);
    void draw();
    void init();
    void consoleCallback(char p_color, std::time_t p_time, const char* p_message);
};