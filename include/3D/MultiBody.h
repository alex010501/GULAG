#pragma once

#include <3D/Entity.h>

class MultiBody: public Entity
{
public:
    MultiBody(const char* p_name, osg::Matrix p_worldTransform, Entity* p_parent = nullptr):
    Entity(p_name, p_worldTransform, p_parent){}

    void update() override {}
    void initPhysics(btDiscreteDynamicsWorld* p_dynamicsWorld) override {}

    const char* getIcon() override { return ICON_FA_GEARS; };
    entityType getEntityType() override { return entityType::MULTIBODY; }

    std::vector<Entity*> getChildren() override { return std::vector<Entity*>(); }
};