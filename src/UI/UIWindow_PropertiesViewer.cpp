#include <UI/UIWindow_PropertiesViewer.h>

UIWindow_PropertiesViewer::UIWindow_PropertiesViewer(const char* p_title): UIWindow(p_title)
{

}

void UIWindow_PropertiesViewer::init()
{
    
}

void UIWindow_PropertiesViewer::draw()
{
    ImGui::Begin(m_title);

    if (this->m_selectedEntity == nullptr || m_selectedEntity->getParent() == nullptr)
    {
        ImGui::End();
        return;
    }
        
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Text("Object: %s", this->m_selectedEntity->getName());
        ImGui::Text("Parent: %s", this->m_selectedEntity->getParent()->getName());
        ImGui::SameLine();
        ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY() - 2));
        if (ImGui::Button("Change"))
        {

        }

        ImGui::Text("Coordinates");
        ImGui::SameLine(0, 12);
        if (ImGui::RadioButton("World", !(this->m_transformLocal)))
            this->m_transformLocal = false;
        ImGui::SameLine();
        if (ImGui::RadioButton("Local", this->m_transformLocal))
            this->m_transformLocal = true;

        if (this->m_transformLocal)
        {
            this->getTranslationAndRotation(this->m_selectedEntity->getLocalTransform()->getMatrix());
        }
        else
        {
            this->getTranslationAndRotation(this->m_selectedEntity->getWorldTransform()->getMatrix());
        }


        this->drawTransformInputs(this->m_translation, this->m_rotation);

        if (this->m_isMoved)
        {
            moveOption lv_moveOption = this->m_transformLocal ? moveOption::LOCAL_ABS : moveOption::WORLD_ABS;
            this->m_selectedEntity->move(lv_moveOption, this->getMatrix());
        }
    }

    if (ImGui::CollapsingHeader("Mesh"))//, ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Text("Mesh");
    }

    if (ImGui::CollapsingHeader("Graphics"))//, ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Text("Graphics");
    }

    if (ImGui::CollapsingHeader("Physics"))//, ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Text("Physics");
    }

    ImGui::End();
}

void UIWindow_PropertiesViewer::drawTransformInputs(double *p_translation, double *p_rotation)
{
    bool lv_xMove, lv_yMove, lv_zMove, lv_aMove, lv_bMove, lv_cMove;

    float lv_textInputWidth = (ImGui::GetWindowWidth() - 110 - ImGui::CalcTextSize("POS:").x) / 3;

    // Position

    ImGui::Text("POS: ");
    ImGui::SameLine(0, 10);
    
    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(255, 0, 0, 255));
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
    ImGui::PushItemWidth(30);
    ImGui::BeginDisabled(true);
    ImGui::Button("X");
    ImGui::EndDisabled();
    ImGui::PopItemWidth();
    ImGui::PopStyleColor(2);

    ImGui::SameLine(0, 0);

    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(255, 255, 255, 255));
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 0, 255));
    ImGui::PushItemWidth(lv_textInputWidth);
    ImGui::InputDouble("##X", p_translation, 0.0, 0.0, "%.3f");
    lv_xMove = ImGui::IsItemEdited();
    ImGui::PopItemWidth();
    ImGui::PopStyleColor(2);

    ImGui::SameLine(0, 10);

    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 255, 0, 255));
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
    ImGui::PushItemWidth(30);
    ImGui::BeginDisabled(true);
    ImGui::Button("Y");
    ImGui::EndDisabled();
    ImGui::PopItemWidth();
    ImGui::PopStyleColor(2);

    ImGui::SameLine(0, 0);

    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(255, 255, 255, 255));
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 0, 255));
    ImGui::PushItemWidth(lv_textInputWidth);
    this->m_isMoved = ImGui::InputDouble("##Y", p_translation + 1, 0.0, 0.0, "%.3f");
    lv_yMove = ImGui::IsItemEdited();
    ImGui::PopItemWidth();
    ImGui::PopStyleColor(2);

    ImGui::SameLine(0, 10);

    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 255, 255));
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
    ImGui::PushItemWidth(30);
    ImGui::BeginDisabled(true);
    ImGui::Button("Z");
    ImGui::EndDisabled();
    ImGui::PopItemWidth();
    ImGui::PopStyleColor(2);

    ImGui::SameLine(0, 0);

    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(255, 255, 255, 255));
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 0, 255));
    ImGui::PushItemWidth(lv_textInputWidth);
    this->m_isMoved = ImGui::InputDouble("##Z", p_translation + 2, 0.0, 0.0, "%.3f");
    lv_zMove = ImGui::IsItemEdited();
    ImGui::PopItemWidth();
    ImGui::PopStyleColor(2);
    
    // Rotation

    ImGui::Text("ROT: ");
    ImGui::SameLine(0, 10);
    
    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(255, 0, 0, 255));
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
    ImGui::PushItemWidth(30);
    ImGui::BeginDisabled(true);
    ImGui::Button("A");
    ImGui::EndDisabled();
    ImGui::PopItemWidth();
    ImGui::PopStyleColor(2);

    ImGui::SameLine(0, 0);

    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(255, 255, 255, 255));
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 0, 255));
    ImGui::PushItemWidth(lv_textInputWidth);
    this->m_isMoved = ImGui::InputDouble("##A", p_rotation, 0.0, 0.0, "%.3f");
    lv_aMove = ImGui::IsItemEdited();
    ImGui::PopItemWidth();
    ImGui::PopStyleColor(2);

    ImGui::SameLine(0, 10);

    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 255, 0, 255));
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
    ImGui::PushItemWidth(30);
    ImGui::BeginDisabled(true);
    ImGui::Button("B");
    ImGui::EndDisabled();
    ImGui::PopItemWidth();
    ImGui::PopStyleColor(2);

    ImGui::SameLine(0, 0);

    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(255, 255, 255, 255));
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 0, 255));
    ImGui::PushItemWidth(lv_textInputWidth);
    this->m_isMoved = ImGui::InputDouble("##B", p_rotation + 1, 0.0, 0.0, "%.3f");
    lv_bMove = ImGui::IsItemEdited();
    ImGui::PopItemWidth();
    ImGui::PopStyleColor(2);

    ImGui::SameLine(0, 10);

    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 255, 255));
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
    ImGui::PushItemWidth(30);
    ImGui::BeginDisabled(true);
    ImGui::Button("C");
    ImGui::EndDisabled();
    ImGui::PopItemWidth();
    ImGui::PopStyleColor(2);

    ImGui::SameLine(0, 0);

    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(255, 255, 255, 255));
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 0, 255));
    ImGui::PushItemWidth(lv_textInputWidth);
    ImGui::InputDouble("##C", p_rotation + 2, 0.0, 0.0, "%.3f");
    lv_cMove = ImGui::IsItemEdited();
    ImGui::PopItemWidth();
    ImGui::PopStyleColor(2);

    this->m_isMoved = lv_xMove || lv_yMove || lv_zMove || lv_aMove || lv_bMove || lv_cMove;
}

void UIWindow_PropertiesViewer::getTransform()
{
    if (this->m_selectedEntity)
    {
        if (this->m_transformLocal)
        {
            osg::Matrix lv_transformMatrix = this->m_selectedEntity->getLocalTransform()->getMatrix();
            this->getTranslationAndRotation(lv_transformMatrix);
        }
        else
        {
            osg::Matrix lv_transformMatrix = this->m_selectedEntity->getWorldTransform()->getMatrix();
            this->getTranslationAndRotation(lv_transformMatrix);
        }
    }
}

void UIWindow_PropertiesViewer::setTransform()
{
    if (this->m_selectedEntity)
    {
        if (this->m_transformLocal)
        {
            osg::Matrix lv_transformMatrix = this->getMatrix();
            this->m_selectedEntity->move(moveOption::LOCAL_ABS, lv_transformMatrix);
        }
        else
        {
            osg::Matrix lv_transformMatrix = this->getMatrix();
            this->m_selectedEntity->move(moveOption::WORLD_ABS, lv_transformMatrix);
        }
    }
}

osg::Matrix UIWindow_PropertiesViewer::getMatrix()
{
    osg::Matrix lv_matrix;

    // Set the translation
    lv_matrix.setTrans(this->m_translation[0], this->m_translation[1], this->m_translation[2]);

    // Set the rotation
    double lv_rotation[3];
    for (int i = 0; i < 3; i++)
        lv_rotation[i] = this->degToRad(this->m_rotation[i]);
    osg::Quat lv_quat = osg::Quat(lv_rotation[0], osg::Vec3(1.0, 0.0, 0.0));
    lv_quat *= osg::Quat(lv_rotation[1], osg::Vec3(0.0, 1.0, 0.0));
    lv_quat *= osg::Quat(lv_rotation[2], osg::Vec3(0.0, 0.0, 1.0));
    lv_matrix.setRotate(lv_quat);

    return lv_matrix;
}

void UIWindow_PropertiesViewer::getTranslationAndRotation(osg::Matrix p_matrix)
{
    double lv_rotation[3] = {0.0, 0.0, 0.0};
    // Get the translation
    osg::Vec3d lv_tr = p_matrix.getTrans();
    this->m_translation[0] = lv_tr.x();
    this->m_translation[1] = lv_tr.y();
    this->m_translation[2] = lv_tr.z();

    // Get the rotation
    // osg::Quat lv_quat = p_matrix.getRotate();
    // lv_quat.getRotate(lv_rotation[0], osg::Vec3(1.0, 0.0, 0.0));
    // lv_quat.getRotate(lv_rotation[1], osg::Vec3(0.0, 1.0, 0.0));
    // lv_quat.getRotate(lv_rotation[2], osg::Vec3(0.0, 0.0, 1.0));
    // Convert the rotation from radians to degrees
    this->m_rotation[0] = this->radToDeg(lv_rotation[0]);
    this->m_rotation[1] = this->radToDeg(lv_rotation[1]);
    this->m_rotation[2] = this->radToDeg(lv_rotation[2]);
}