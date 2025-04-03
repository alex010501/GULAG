#pragma once

#include <iostream>
#include <string>
#include <cstring>

#include <stb_image.h>
#include <assimp/Importer.hpp>

#include <osgViewer/Viewer>
#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/PositionAttitudeTransform>
#include <osg/MatrixTransform>
#include <osgGA/FirstPersonManipulator>
#include <osg/Texture2D>

#include <btBulletDynamicsCommon.h>

// #include <3D/Objects/BaseObject.h>
#include <3D/Entity.h>
#include <3D/Dummy.h>
#include <3D/Folder.h>
#include <3D/Object.h>
#include <3D/MultiBody.h>

class Scene
{
private:
    // Scene name
    std::string m_sceneName;

    // Bullet Physics
    btDiscreteDynamicsWorld* m_dynamicsWorld;
    btAxisSweep3* m_broadphase;
    btDefaultCollisionConfiguration* m_collisionConfiguration;
    btCollisionDispatcher* m_dispatcher;
    btSequentialImpulseConstraintSolver* m_solver;
    
    // Base Visualization
    osg::ref_ptr<osg::ShapeDrawable> m_floor;
    osg::ref_ptr<osg::Group> m_XYZAxes;
    osg::ref_ptr<osg::ShapeDrawable> createFloor();
    osg::ref_ptr<osg::Group> createAxes();

    // Base physics
    btRigidBody* m_groundbody;
    btRigidBody* createGround();

    // Entities system
    SceneRoot m_sceneRoot;// Root of the scene
    Entity* m_selectedEntity;  
    
protected:
    osg::ref_ptr<osg::Group> m_OSGroot;

public:
    Scene();
    ~Scene();

    osg::ref_ptr<osg::Group> getOSGRoot(){return m_OSGroot.get();}

    Entity* getSceneRoot(){return &m_sceneRoot;}

    void initPhysics();

    void update(double p_dt);

    void addEntity(EntityInfo p_info);

    void selectEntity(Entity* p_entity) { m_selectedEntity = p_entity; std::cout << "Selected entity: " << p_entity->getName() << std::endl; }
};