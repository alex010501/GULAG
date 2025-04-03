#include <UI/UIWindow_Console.h>
#include <chrono>
#include <iostream>

UIWindow_Console::UIWindow_Console(const char* p_title): UIWindow(p_title)
{
    this->m_commandBuffer[0] = '\0';
}

void UIWindow_Console::init()
{
    
}

void UIWindow_Console::draw()
{
    ImGui::Begin(m_title);

    ImVec2 size = ImGui::GetContentRegionAvail();

    // ImGui::Scrollbar(ImGui::Axis_X);

    ImGui::BeginChildFrame(ImGui::GetID("ConsoleFrame"), ImVec2(size.x, size.y - 25), ImGuiWindowFlags_AlwaysVerticalScrollbar);

    for (consoleMessage message : this->m_consoleLines)
    {
        this->drawConsoleItem(message.color, message.message);
    }

    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::EndChildFrame();

    ImGui::PushItemWidth(size.x - 150);
    ImGui::InputText(" ", m_commandBuffer, IM_ARRAYSIZE(m_commandBuffer));
    ImGui::PopItemWidth();
    ImGui::SameLine();

    ImGui::PushItemWidth(100);
    if (ImGui::Button("Enter"))
    {
        comandCallback();
    }
    ImGui::PopItemWidth();

    ImGui::SameLine();

    ImGui::PushItemWidth(100);
    if (ImGui::Button("Save log"))
    {

    }
    ImGui::PopItemWidth();

    ImGui::End();
}

void UIWindow_Console::drawConsoleItem(char p_color, const char* p_message)
{
    switch (p_color)
    {
        case 'r':
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", p_message);
            break;
        case 'g':
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "%s", p_message);
            break;
        case 'w':
            ImGui::Text("%s", p_message);
            break;
        case 'y':
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "%s", p_message);
            break;
        case 'c':
            ImGui::TextColored(ImVec4(0, 1, 1, 1), "%s", p_message);
            break;
        default:
            ImGui::Text("%s", p_message);
            break;
    };
}

void UIWindow_Console::comandCallback()
{
    if (this->m_commandBuffer[0] == '\0')
        return;
    this->consoleCallback('c', std::time(0), this->m_commandBuffer);
    this->m_commandBuffer[0] = '\0';
}

void UIWindow_Console::consoleCallback(char p_color, std::time_t p_time, const char* p_message)
{ 
    char timeStr[20];
    std::strftime(timeStr, sizeof(timeStr), "[%H:%M:%S]", std::localtime(&p_time));

    std::string lv_buffer;
    lv_buffer += timeStr;
    switch (p_color)
    {
        case 'r':
            lv_buffer += "-[ERROR]-";
            break;
        case 'g':
            lv_buffer += "-[SUCCESS]-";
            break;
        case 'w':
            lv_buffer += "-[INFO]-";
            break;
        case 'y':
            lv_buffer += "-[WARNING]-";
            break;
        case 'c':
            lv_buffer += "-[COMMAND]-";
            break;
        default:
            lv_buffer += "-[UNKNOWN]-";
            break;
    }
    lv_buffer += p_message;
    lv_buffer += "\n";

    consoleMessage lv_message;
    lv_message.color = p_color;
    strcpy(lv_message.message, lv_buffer.c_str());
    this->m_consoleLines.push_back(lv_message);
}