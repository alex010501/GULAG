#include <3D/Object.h>

Object::Object(const char* p_name, osg::Matrix p_worldTransform, Entity* p_parent, const char* p_modelPath):
Entity(p_name, p_worldTransform, p_parent)
{    
    this->m_mesh = new osg::Geode;
    this->m_localTransform->addChild(m_mesh.get());
    this->m_isMeshLoaded = false;
    this->m_isPhysicsEnabled = false;
    this->m_objectType = ObjectType::GHOST_OBJECT;
    this->m_modelPath = "";
    this->m_color = osg::Vec4(1.0, 1.0, 1.0, 1.0);
    this->m_shape = nullptr;
    this->m_motionState = nullptr;
    this->m_rigidBody = nullptr;
    this->m_inertia = btVector3(0, 0, 0);
    this->m_mass = 0.0;

    if (p_modelPath != nullptr)
    {
        this->setMesh(p_modelPath);
    }
}

Object::~Object()
{
    this->deleteRigidBody();
}

void Object::initPhysics(btDiscreteDynamicsWorld* p_dynamicsWorld)
{
    if (this->m_isPhysicsEnabled)
    {
        p_dynamicsWorld->addRigidBody(this->m_rigidBody);
    }
}

void Object::update()
{
    
}

void Object::changeColor()
{
    if (this->m_isMeshLoaded)
    {
        osg::ref_ptr<osg::Material> material = new osg::Material;
        material->setDiffuse(osg::Material::FRONT_AND_BACK, this->m_color);
        this->m_mesh->getOrCreateStateSet()->setAttributeAndModes(material.get());
        this->m_mesh->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::ON);
    }
}

btTransform Object::OSG2BT_Transform(osg::ref_ptr<osg::MatrixTransform> p_transform)
{
    osg::Matrix lv_matrix = p_transform->getMatrix();
    btMatrix3x3 lv_rotation = btMatrix3x3(lv_matrix(0, 0), lv_matrix(0, 1), lv_matrix(0, 2),
                                          lv_matrix(1, 0), lv_matrix(1, 1), lv_matrix(1, 2),
                                          lv_matrix(2, 0), lv_matrix(2, 1), lv_matrix(2, 2));
    btVector3 lv_position = btVector3(lv_matrix(0, 3), lv_matrix(1, 3), lv_matrix(2, 3));
    btTransform lv_transform(lv_rotation, lv_position);
    return lv_transform;
}

osg::Matrix Object::BT2OSG_Transform(btTransform p_transform)
{
    btMatrix3x3 lv_rotation = btMatrix3x3(p_transform.getBasis());
    btVector3 lv_position = p_transform.getOrigin();
    osg::Matrix lv_matrix = osg::Matrix(lv_rotation[0][0], lv_rotation[0][1], lv_rotation[0][2], lv_position[0],
                                        lv_rotation[1][0], lv_rotation[1][1], lv_rotation[1][2], lv_position[1],
                                        lv_rotation[2][0], lv_rotation[2][1], lv_rotation[2][2], lv_position[2],
                                                        0,                 0,                 0,              1);
    return lv_matrix;
}

void Object::deleteRigidBody()
{
    if (this->m_rigidBody)
        delete this->m_rigidBody;
    if (this->m_shape)
        delete this->m_shape;
    if (this->m_motionState)
        delete this->m_motionState;
}

void Object::setRigidBody()
{
    Assimp::Importer lv_importer;
    const aiScene* lv_scene = lv_importer.ReadFile(this->m_modelPath, aiProcess_Triangulate);

    // Convert mesh data to Bullet format
    btTriangleMesh* lv_triangleMesh = new btTriangleMesh();
    aiMesh* lv_mesh = lv_scene->mMeshes[0];
    for (unsigned int i = 0; i < lv_mesh->mNumFaces; i++)
    {
        aiFace lv_face = lv_mesh->mFaces[i];
        btVector3 v1 = btVector3(lv_mesh->mVertices[lv_face.mIndices[0]].x, lv_mesh->mVertices[lv_face.mIndices[0]].y, lv_mesh->mVertices[lv_face.mIndices[0]].z);
        btVector3 v2 = btVector3(lv_mesh->mVertices[lv_face.mIndices[1]].x, lv_mesh->mVertices[lv_face.mIndices[1]].y, lv_mesh->mVertices[lv_face.mIndices[1]].z);
        btVector3 v3 = btVector3(lv_mesh->mVertices[lv_face.mIndices[2]].x, lv_mesh->mVertices[lv_face.mIndices[2]].y, lv_mesh->mVertices[lv_face.mIndices[2]].z);
        lv_triangleMesh->addTriangle(v1, v2, v3);
    }

    this->m_shape = new btBvhTriangleMeshShape(lv_triangleMesh, true);

    this->m_motionState = new btDefaultMotionState(OSG2BT_Transform(this->getWorldTransform()));

    this->m_shape->calculateLocalInertia(this->m_mass, this->m_inertia);
    btRigidBody::btRigidBodyConstructionInfo lv_info(this->m_mass, this->m_motionState, this->m_shape, this->m_inertia);

    this->m_rigidBody = new btRigidBody(lv_info);
}

void Object::setMesh(const char* p_modelPath)
{
    // osgDB::Registry::instance()->loadLibrary("osgdb_osg");
    // osgDB::Registry::instance()->loadLibrary("osgdb_obj");
    // osgDB::Registry::instance()->loadLibrary("osgdb_stl");
    osg::ref_ptr<osg::Node> lv_node = osgDB::readNodeFile(p_modelPath);
    if (lv_node)
    {
        this->m_mesh->addChild(lv_node.get());
        this->m_modelPath = p_modelPath;
        this->m_isMeshLoaded = true;
        this->changeColor();

        switch (this->m_objectType)
        {
        case ObjectType::GHOST_OBJECT:
            this->deleteRigidBody();
            break;
        case ObjectType::STATIC_OBJECT:
            this->setMass(0.0);
            break;
        case ObjectType::DYNAMIC_OBJECT:
            this->setMass(1.0);
            break;        
        default:
            break;
        }
    }
    else
        std::cout << "Failed to load model: " << p_modelPath << std::endl;
}

void Object::setMass(double p_mass)
{
    this->m_mass = p_mass;
    this->setRigidBody();
}

void Object::setObjectType(ObjectType p_objectType)
{
    if (!this->m_isMeshLoaded)
        return;

    
    this->m_objectType = p_objectType;

    switch (this->m_objectType)
    {
        case ObjectType::GHOST_OBJECT:
            this->m_isPhysicsEnabled = false;
            this->deleteRigidBody();
            break;
        case ObjectType::STATIC_OBJECT:
            this->m_isPhysicsEnabled = true;
            this->setMass(0.0);
            break;
        case ObjectType::DYNAMIC_OBJECT:
            this->m_isPhysicsEnabled = true;
            this->setMass(1.0);
            break;
        default:
            break;
    }
}

CubeObject::CubeObject(const char* p_name, double a, double b, double c, osg::Matrix p_worldTransform, Entity* p_parent):
Object(p_name, p_worldTransform, p_parent)
{
    this->m_a = a;
    this->m_b = b;
    this->m_c = c;
    this->resize();

    this->setColor(COLOR_WHITE);
    this->setTransparency(1.0);
}

void CubeObject::resize()
{
    this->m_mesh->removeDrawables(0, this->m_mesh->getNumDrawables());
    this->m_mesh->addDrawable(new osg::ShapeDrawable(new osg::Box(osg::Vec3(0, 0, 0), this->m_a, this->m_b, this->m_c)));

    switch (this->getObjectType())
    {
    case ObjectType::GHOST_OBJECT:
        this->deleteRigidBody();
        break;
    case ObjectType::STATIC_OBJECT:
        this->setMass(0.0);
        break;
    case ObjectType::DYNAMIC_OBJECT:
        this->setMass(1.0);
        break;    
    default:
        break;
    }
}

void CubeObject::setRigidBody()
{
    this->m_shape = new btBoxShape(btVector3(this->m_a, this->m_b, this->m_c));
    this->m_motionState = new btDefaultMotionState(OSG2BT_Transform(this->getWorldTransform()));
    this->m_shape->calculateLocalInertia(this->m_mass, this->m_inertia);
    btRigidBody::btRigidBodyConstructionInfo lv_info(this->m_mass, this->m_motionState, this->m_shape, this->m_inertia);

    this->m_rigidBody = new btRigidBody(lv_info);
}


SphereObject::SphereObject(const char* p_name, double p_radius, osg::Matrix p_worldTransform, Entity* p_parent):
Object(p_name, p_worldTransform, p_parent)
{
    this->m_radius = p_radius;
    this->resize();

    this->setColor(COLOR_WHITE);
    this->setTransparency(1.0);
}

void SphereObject::resize()
{
    this->m_mesh->removeDrawables(0, this->m_mesh->getNumDrawables());
    this->m_mesh->addDrawable(new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0, 0, 0), this->m_radius)));

    switch (this->getObjectType())
    {
    case ObjectType::GHOST_OBJECT:
        this->deleteRigidBody();
        break;
    case ObjectType::STATIC_OBJECT:
        this->setMass(0.0);
        break;
    case ObjectType::DYNAMIC_OBJECT:
        this->setMass(1.0);
        break;    
    default:
        break;
    }
}

void SphereObject::setRigidBody()
{
    this->m_shape = new btSphereShape(this->m_radius);
    this->m_motionState = new btDefaultMotionState(OSG2BT_Transform(this->getWorldTransform()));
    this->m_shape->calculateLocalInertia(this->m_mass, this->m_inertia);
    btRigidBody::btRigidBodyConstructionInfo lv_info(this->m_mass, this->m_motionState, this->m_shape, this->m_inertia);

    this->m_rigidBody = new btRigidBody(lv_info);
}


CylinderObject::CylinderObject(const char* p_name, double p_radius, double p_height, osg::Matrix p_worldTransform, Entity* p_parent):
Object(p_name, p_worldTransform, p_parent)
{
    this->m_radius = p_radius;
    this->m_height = p_height;
    this->resize();

    this->setColor(COLOR_WHITE);
    this->setTransparency(1.0);
}

void CylinderObject::resize()
{
    this->m_mesh->removeDrawables(0, this->m_mesh->getNumDrawables());
    this->m_mesh->addDrawable(new osg::ShapeDrawable(new osg::Cylinder(osg::Vec3(0, 0, 0), this->m_radius, this->m_height)));

    switch (this->getObjectType())
    {
    case ObjectType::GHOST_OBJECT:
        this->deleteRigidBody();
        break;
    case ObjectType::STATIC_OBJECT:
        this->setMass(0.0);
        break;
    case ObjectType::DYNAMIC_OBJECT:
        this->setMass(1.0);
        break;    
    default:
        break;
    }
}

void CylinderObject::setRigidBody()
{
    this->m_shape = new btCylinderShape(btVector3(this->m_radius, this->m_radius, this->m_height));
    this->m_motionState = new btDefaultMotionState(OSG2BT_Transform(this->getWorldTransform()));
    this->m_shape->calculateLocalInertia(this->m_mass, this->m_inertia);
    btRigidBody::btRigidBodyConstructionInfo lv_info(this->m_mass, this->m_motionState, this->m_shape, this->m_inertia);

    this->m_rigidBody = new btRigidBody(lv_info);
}