#pragma once

#include <UI/UIWindow.h>

#include <3D/Entity.h>


class UIWindow_AddEntityDialog: public UIWindow {
private:
    // Dimensions input
    double m_a;
    double m_b;
    double m_c;
    // Mesh input
    // const char* m_mesh_file;
    char m_mesh_file_buffer[256];

    // Name input
    char m_entityName[256];

    // Draw or not
    bool m_draw;
    // Entity type
    entityType m_entityType;

    void resetValues();

public:
    // Add variables here
    sigslot::signal1<EntityInfo> signal_CreateEntity;

    // Add class methods here
    UIWindow_AddEntityDialog(const char* p_title);

    void show() { this->m_draw = true; }
    void setEntityType(entityType p_entityType){m_entityType = p_entityType;}

    void draw();
    void init();
};