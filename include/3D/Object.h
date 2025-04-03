#pragma once

#include <config.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <osg/Material>
#include <osg/ShapeDrawable>
#include <osg/Texture2D>
#include <osgDB/ReadFile>

#ifdef B_DEBUG

#endif

#ifdef B_RELEASE
USE_OSGPLUGIN(osg)
USE_OSGPLUGIN(obj)
USE_OSGPLUGIN(stl)
#endif

#include <3D/Entity.h>

#define COLOR_WHITE   osg::Vec4(1.0, 1.0, 1.0, 1.0)
#define COLOR_BLACK   osg::Vec4(0.0, 0.0, 0.0, 1.0)
#define COLOR_RED     osg::Vec4(1.0, 0.0, 0.0, 1.0)
#define COLOR_YELLOW  osg::Vec4(1.0, 1.0, 0.0, 1.0)
#define COLOR_GREEN   osg::Vec4(0.0, 1.0, 0.0, 1.0)
#define COLOR_CYAN    osg::Vec4(0.0, 1.0, 1.0, 1.0)
#define COLOR_BLUE    osg::Vec4(0.0, 0.0, 1.0, 1.0)
#define COLOR_MAGENTA osg::Vec4(1.0, 0.0, 1.0, 1.0)

enum ObjectColor
{
    WHITE = 0,
    BLACK = 1,
    RED = 2,
    YELLOW = 3,
    GREEN = 4,
    CYAN = 5,
    BLUE = 6,
    MAGENTA = 7
};

enum ObjectType
{
    GHOST_OBJECT = 0,
    STATIC_OBJECT = 1,
    DYNAMIC_OBJECT = 2
};

class Object: public Entity
{
private:
    bool m_isMeshLoaded;
    bool m_isPhysicsEnabled;

    ObjectType m_objectType;
    const char* m_modelPath;

    void changeColor();
protected:
    osg::ref_ptr<osg::Geode> m_mesh;
    osg::Vec4 m_color;

    btCollisionShape* m_shape;
    btDefaultMotionState* m_motionState;
    btRigidBody* m_rigidBody;
    btVector3 m_inertia;
    double m_mass;

    btTransform OSG2BT_Transform(osg::ref_ptr<osg::MatrixTransform> p_transform);
    osg::Matrix BT2OSG_Transform(btTransform p_transform);
    
    void deleteRigidBody();
    void setRigidBody();

public:
    Object(const char* p_name, osg::Matrix p_worldTransform, Entity* p_parent, const char* p_modelPath = nullptr);
    ~Object();

    const char* getIcon() override { return ICON_FA_SHIELD_HALVED; }
    entityType getEntityType() override { return entityType::OBJECT; }
    const char* getModelPath() { return m_modelPath; }
    osg::Vec4 getColor() { return m_color; }
    double getTransparency() { return m_color.w(); }
    double getMass() { return m_mass; }
    ObjectType getObjectType() { return m_objectType; }

    void setMesh(const char* p_modelPath);
    void setColor(osg::Vec4 p_color) { m_color.x() = p_color.x(); m_color.y() = p_color.y(); m_color.z() = p_color.z(); changeColor(); }
    void setTransparency(double p_alpha) { m_color.w() = p_alpha; changeColor(); }
    void setMass(double p_mass);
    void setObjectType(ObjectType p_objectType);

    void update() override;

    void initPhysics(btDiscreteDynamicsWorld* p_dynamicsWorld) override;

    std::vector<Entity*> getChildren() override { return std::vector<Entity*>(); }
};

class CubeObject: public Object
{
private:
    double m_a;
    double m_b;
    double m_c;

    void resize();
    void setRigidBody();
public:
    CubeObject(const char* p_name, double a, double b, double c, osg::Matrix p_worldTransform, Entity* p_parent);

    const char* getIcon() override { return ICON_FA_CUBE; }
    entityType getEntityType() override { return entityType::CUBE; }

    double getA() { return m_a; }
    double getB() { return m_b; }
    double getC() { return m_c; }

    void setA(double p_a) { m_a = p_a; resize(); }
    void setB(double p_b) { m_b = p_b; resize(); }
    void setC(double p_c) { m_c = p_c; resize(); }
};

class SphereObject: public Object
{
private:
    double m_radius;

    void resize();
    void setRigidBody();
public:
    SphereObject(const char* p_name, double p_radius, osg::Matrix p_worldTransform, Entity* p_parent);

    const char* getIcon() override { return ICON_FA_GLOBE; }
    entityType getEntityType() override { return entityType::SPHERE; }

    double getRadius() { return m_radius; }

    void setRadius(double p_radius) { m_radius = p_radius; resize(); }
};

class CylinderObject: public Object
{
private:
    double m_radius;
    double m_height;

    void resize();
    void setRigidBody();
public:
    CylinderObject(const char* p_name, double p_radius, double p_height, osg::Matrix p_worldTransform, Entity* p_parent);

    const char* getIcon() override { return ICON_FA_DATABASE; }
    entityType getEntityType() override { return entityType::CYLINDER; }

    double getRadius() { return m_radius; }
    double getHeight() { return m_height; }

    void setRadius(double p_radius) { m_radius = p_radius; resize(); }
    void setHeight(double p_height) { m_height = p_height; resize(); }
};