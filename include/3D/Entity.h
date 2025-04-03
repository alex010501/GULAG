#pragma once

#include <iostream>

#include <vector>
#include <algorithm>

#include <osg/Geode>
#include <osg/MatrixTransform>

#include <btBulletDynamicsCommon.h>

#include <UI/IconsFontAwesome6.h>

// #include <3D/Objects/ObejctsHelper.h>

enum entityType
{
    ENTITY = 0,    
    
    DUMMY  = 10,

    FOLDER = 20,
    SCENEROOT = 21,
    
    OBJECT = 30,
    CUBE   = 31,
    SPHERE = 32,
    CYLINDER = 33,
    
    MULTIBODY = 40
};

enum moveOption
{
    LOCAL_ABS = 0,
    LOCAL_REL = 1,
    WORLD_ABS = 2,
    WORLD_REL = 3
};

class Entity
{
private:
    std::string m_name;

protected:
    // Hierarchy variables
    Entity* m_parent;

    // OSG variables
    osg::ref_ptr<osg::MatrixTransform> m_localTransform;

public:
    Entity(const char* p_name, osg::Matrix p_worldTransform, Entity* p_parent = nullptr);

    virtual ~Entity() {};

    virtual void update() = 0;

    virtual void initPhysics(btDiscreteDynamicsWorld* p_dynamicsWorld) = 0;

    const char* getName(){return m_name.c_str(); }
    void rename(const char* p_name){ m_name = p_name; }

    virtual entityType getEntityType(){ return entityType::ENTITY; }

    virtual const char* getIcon(){ return ICON_FA_ADDRESS_CARD; }

    Entity* getParent(){ return m_parent; }
    void setParent(Entity* p_parent);

    virtual std::vector<Entity*> getChildren() = 0;

    osg::ref_ptr<osg::MatrixTransform> getWorldTransform();

    osg::ref_ptr<osg::MatrixTransform> getLocalTransform();

    void move(moveOption p_moveOption, osg::Matrix p_transform);
};

struct EntityInfo
{
    entityType type = entityType::ENTITY;
    const char* name = nullptr;
    osg::Matrix worldTransform = osg::Matrix::identity();
    Entity* parent = nullptr;

    const char* modelPath = nullptr;
    
    double a = 1.0;
    double b = 1.0;
    double c = 1.0;
};