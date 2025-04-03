#include <UI/UIWindow_LibraryViewer.h>

UIWindow_LibraryViewer::UIWindow_LibraryViewer(const char* p_title): UIWindow(p_title)
{

}

void UIWindow_LibraryViewer::init()
{
    
}

void UIWindow_LibraryViewer::draw()
{
    ImGui::Begin(m_title);
    ImGui::Button("Library Viewer");
    ImGui::End();
}