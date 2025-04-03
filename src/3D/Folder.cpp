#include <3D/Folder.h>


Folder::Folder(const char* p_name, osg::Matrix p_worldTransform, Entity* p_parent):
Entity(p_name, p_worldTransform, p_parent)
{

}

Folder::~Folder()
{
    for (auto& child : this->m_children)
    {
        delete child;
    }
    this->m_children.clear();
}

void Folder::update()
{
    for (auto& child : this->m_children)
    {
        child->update();
    }
}

void Folder::initPhysics(btDiscreteDynamicsWorld* p_dynamicsWorld)
{
    for (auto& child : this->m_children)
    {
        child->initPhysics(p_dynamicsWorld);
    }
}

void Folder::addChild(Entity* p_child)
{
    this->m_children.push_back(p_child);
    this->m_localTransform->addChild(p_child->getLocalTransform());
}

void Folder::removeChild(Entity* p_child)
{
    this->m_children.erase(
        std::remove(this->m_children.begin(), this->m_children.end(), p_child),
        this->m_children.end()
    );
    this->m_localTransform->removeChild(p_child->getLocalTransform());
}

std::vector<Entity*> Folder::getChildren()
{
    return this->m_children;
}

SceneRoot::SceneRoot(const char* p_name):
Folder(p_name, osg::Matrix::identity(), nullptr)
{

}

SceneRoot::SceneRoot():
Folder("Scene root", osg::Matrix::identity(), nullptr)
{

}