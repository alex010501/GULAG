#include <UI/UIWindow_SceneTreeViewer.h>

SceneTreeObject::SceneTreeObject(Entity* p_entity, UIWindow* p_parent):
m_parent(p_parent),
m_entity(p_entity)
{
    this->m_iconName = p_entity->getIcon();
    this->m_iconName += " ";
    this->m_iconName += p_entity->getName();
    if (p_entity->getEntityType() == entityType::FOLDER || p_entity->getEntityType() == entityType::SCENEROOT)
        this->m_hasChildren = true;
    else
        this->m_hasChildren = false;
    
    this->m_isSelected = false;
    this->m_isOpen = false;
}

void SceneTreeObject::draw()
{
    if (this->m_hasChildren)
    {
        this->m_isOpen = ImGui::TreeNodeEx(this->m_iconName.c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Selected);
        if (ImGui::IsItemClicked())
        {
            this->m_isSelected = true;
            static_cast<UIWindow_SceneTreeViewer*>(this->m_parent)->selectEntity(this->m_entity);
        }
        if (this->m_isOpen)
        {
            for (auto child : this->m_entity->getChildren())
            {
                SceneTreeObject *sceneTreeObject = new SceneTreeObject(child, this->m_parent);
                sceneTreeObject->draw();
            }
            ImGui::TreePop();
        }
    }
    else
    {
        if (ImGui::Selectable(this->m_iconName.c_str(), this->m_isSelected))
        {
            this->m_isSelected = true;
            static_cast<UIWindow_SceneTreeViewer*>(this->m_parent)->selectEntity(this->m_entity);
            // this->m_parent->selectEntity(this->m_entity);
        }
    }
}

UIWindow_SceneTreeViewer::UIWindow_SceneTreeViewer(const char* p_title):
UIWindow(p_title)
{

}

void UIWindow_SceneTreeViewer::init()
{
    
}

void UIWindow_SceneTreeViewer::draw()
{
    ImGui::Begin(m_title);

    ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(0, 255, 127, 255));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(0, 255, 0, 255));
    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 127, 0, 255));
    if(ImGui::Button(ICON_FA_CIRCLE_PLUS " Add entity"))
    {
        ImGui::OpenPopup("AddEntityPopup");
    }
    ImGui::PopStyleColor(3);

    if(ImGui::BeginPopup("AddEntityPopup"))
    {
        if (ImGui::Button("Add Dummy"))
        {
            this->signal_showEntityWindow(entityType::DUMMY);
            std::cout << "Adding Dummy" << std::endl;
            ImGui::ClosePopupToLevel(0, true);
        }
        if (ImGui::Button("Add Folder"))
        {
            this->signal_showEntityWindow(entityType::FOLDER);
            std::cout << "Adding Folder" << std::endl;
            ImGui::ClosePopupToLevel(0, true);
        }
        if (ImGui::Button("Add Object"))
        {
            ImGui::OpenPopup("AddObjectPopup");
        }
        if (ImGui::Button("Add MultiBody"))
        {
            this->signal_showEntityWindow(entityType::MULTIBODY);
            std::cout << "Adding MultiBody" << std::endl;
            ImGui::ClosePopupToLevel(0, true);
        }

        if (ImGui::BeginPopup("AddObjectPopup"))
        {
            if (ImGui::Button("Add Mesh"))
            {
                this->signal_showEntityWindow(entityType::OBJECT);
                std::cout << "Adding Mesh" << std::endl;
                ImGui::ClosePopupToLevel(0, true);
            }
            if (ImGui::Button("Add Cube"))
            {
                this->signal_showEntityWindow(entityType::CUBE);
                std::cout << "Adding Cube" << std::endl;
                ImGui::ClosePopupToLevel(0, true);
            }
            if (ImGui::Button("Add Sphere"))
            {
                this->signal_showEntityWindow(entityType::SPHERE);
                std::cout << "Adding Sphere" << std::endl;
                ImGui::ClosePopupToLevel(0, true);
            }
            if (ImGui::Button("Add Cylinder"))
            {
                this->signal_showEntityWindow(entityType::CYLINDER);
                std::cout << "Adding Cylinder" << std::endl;
                ImGui::ClosePopupToLevel(0, true);
            }

            ImGui::EndPopup();
        }        

        ImGui::EndPopup();
    }

    this->m_sceneTreeRoot->draw();

    ImGui::End();
}

void UIWindow_SceneTreeViewer::selectEntity(Entity* p_entity)
{
    this->signal_selectEntity(p_entity);
}