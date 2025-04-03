#pragma once

#include <3D/Entity.h>

class Folder: public Entity
{
private:
    std::vector<Entity*> m_children;

public:
    Folder(const char* p_name, osg::Matrix p_worldTransform, Entity* p_parent = nullptr);
    ~Folder();

    const char* getIcon() override { return ICON_FA_FOLDER; }
    entityType getEntityType() override { return entityType::FOLDER; }

    void update() override;
    void initPhysics(btDiscreteDynamicsWorld* p_dynamicsWorld) override;
    void addChild(Entity* p_entity);
    void removeChild(Entity* p_entity);
    std::vector<Entity*> getChildren() override;
};

class SceneRoot: public Folder
{
public:
    SceneRoot();
    SceneRoot(const char* p_name);

    const char* getIcon() override { return ICON_FA_HASHTAG; }
    entityType getEntityType() override { return entityType::SCENEROOT; }
};