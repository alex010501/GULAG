#pragma once 

#include <3D/Entity.h>

class Dummy: public Entity
{
public:
    Dummy(const char* p_name, osg::Matrix p_worldTransform, Entity* p_parent = nullptr):
    Entity(p_name, p_worldTransform, p_parent){}

    ~Dummy(){}

    void update() override {}
    void initPhysics(btDiscreteDynamicsWorld* p_dynamicsWorld) override {}

    const char* getIcon() override { return ICON_FA_CIRCLE_DOT; };
    entityType getEntityType() override { return entityType::DUMMY; }

    std::vector<Entity*> getChildren() override { return std::vector<Entity*>(); }
};