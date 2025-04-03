#pragma once

#include <UI/UIWindow.h>

#include <3D/Entity.h>

class SceneTreeObject: public sigslot::has_slots<>
{
private:
    UIWindow* m_parent;
    Entity* m_entity;
    std::string m_iconName;
    bool m_hasChildren;
    bool m_isSelected;
    bool m_isOpen;
public:
    SceneTreeObject(Entity* p_entity, UIWindow* p_parent);

    void draw();
};

class UIWindow_SceneTreeViewer: public UIWindow {
private:
    // Entity* m_sceneRoot;
    SceneTreeObject* m_sceneTreeRoot;

public:
    // Add variables here
    sigslot::signal1<entityType> signal_showEntityWindow;
    
    sigslot::signal1<Entity*> signal_selectEntity;

    // Add class methods here
    UIWindow_SceneTreeViewer(const char* p_title);
    void draw();    
    void init();

    void setSceneRoot(Entity* p_sceneRoot) { m_sceneTreeRoot = new SceneTreeObject(p_sceneRoot, this); }
    void selectEntity(Entity* p_entity);
};