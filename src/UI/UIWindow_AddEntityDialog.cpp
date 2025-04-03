#include <UI/UIWindow_AddEntityDialog.h>
#include <tinyfiledialogs.h>

UIWindow_AddEntityDialog::UIWindow_AddEntityDialog(const char* p_title): UIWindow(p_title)
{
    // Set initial values
    this->resetValues();
}

void UIWindow_AddEntityDialog::init()
{
    
}

void UIWindow_AddEntityDialog::resetValues()
{
    // Set initial values
    this->m_a = 1.0;
    this->m_b = 1.0;
    this->m_c = 1.0;
    // this->m_mesh_file = nullptr;
    this->m_mesh_file_buffer[0] = '\0';
    this->m_entityName[0] = '\0';

    // Set draw or not
    this->m_draw = false; // Set to false by default
    // Set the entity type
    this->m_entityType = entityType::ENTITY;
}

void UIWindow_AddEntityDialog::draw()
{
    if (!this->m_draw)
        return;

    ImGui::Begin(m_title, nullptr, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    switch (this->m_entityType)
    {
    case ENTITY:
        ImGui::Text("Error: No entity type selected");
        break;
    
    case FOLDER:
        ImGui::Text(ICON_FA_FOLDER " Folder creating");
        break;

    case DUMMY:
        ImGui::Text(ICON_FA_CIRCLE_DOT " Dummy creating");
        break;

    case OBJECT:
        ImGui::Text(ICON_FA_SHIELD_HALVED " Mesh object creating");
        break;

    case CUBE:
        ImGui::Text(ICON_FA_CUBE " Cube creating");
        break;

    case SPHERE:
        ImGui::Text(ICON_FA_GLOBE " Sphere creating");
        break;

    case CYLINDER:
        ImGui::Text(ICON_FA_DATABASE " Cylinder creating");
        break;

    case MULTIBODY:
        ImGui::Text(ICON_FA_GEARS " MultiBody creating");
        break;

    default:
        break;
    }
    
    ImGui::InputText("Entity name", this->m_entityName, IM_ARRAYSIZE(m_entityName));

    if (this->m_entityType == OBJECT)
    {
        ImGui::InputText("Mesh file", this->m_mesh_file_buffer, IM_ARRAYSIZE(m_mesh_file_buffer));
        ImGui::SameLine(0, 10);
        if (ImGui::Button(ICON_FA_ELLIPSIS))
        {
            const char *lv_extension[] = {"*.obj", "*.stl", "*.osg"};
            const char *lv_filename = tinyfd_openFileDialog("Select a file", "", 3, lv_extension, "Mesh files (.obj, .stl, .osg)", 0);

            if (lv_filename != NULL)
            {
                strcpy(this->m_mesh_file_buffer, lv_filename);
            }
        }
    }

    if (this->m_entityType == CUBE)
    {
        ImGui::Text("Cube dimensions:");
        ImGui::InputDouble("a", &this->m_a);
        ImGui::InputDouble("b", &this->m_b);
        ImGui::InputDouble("c", &this->m_c);
    }

    if (this->m_entityType == CYLINDER)
    {
        ImGui::Text("Cylinder dimensions:");
        ImGui::InputDouble("radius", &this->m_a);
        ImGui::InputDouble("height", &this->m_b);
    }

    if (this->m_entityType == SPHERE)
    {
        ImGui::Text("Sphere radius:");
        ImGui::InputDouble("radius", &this->m_a);
    }

    if (ImGui::Button("Create"))
    {
        EntityInfo entityInfo;
        entityInfo.parent = nullptr;
        entityInfo.name = this->m_entityName;
        entityInfo.type = this->m_entityType;
        entityInfo.modelPath = this->m_mesh_file_buffer;
        entityInfo.a = this->m_a;
        entityInfo.b = this->m_b;
        entityInfo.c = this->m_c;
        entityInfo.worldTransform = osg::Matrix::identity();
        this->signal_CreateEntity(entityInfo);

        this->resetValues();
    }
    ImGui::SameLine(0, 50);
    if (ImGui::Button("Cancel"))
    {
        this->resetValues();
    }    

    ImGui::End();
}