#include <3D/Entity.h>

Entity::Entity(const char* p_name, osg::Matrix p_worldTransform, Entity* p_parent):
m_name(p_name), m_parent(p_parent)
{
    if(this->m_parent)
    {
        osg::ref_ptr<osg::MatrixTransform> lv_parentTransform = this->m_parent->getWorldTransform();
        osg::Matrix lv_localTransformMatrix = lv_parentTransform->getInverseMatrix() * p_worldTransform;
        this->m_localTransform = new osg::MatrixTransform(lv_localTransformMatrix);
    }
    else
        this->m_localTransform = new osg::MatrixTransform(p_worldTransform);
}

osg::ref_ptr<osg::MatrixTransform> Entity::getWorldTransform()
{
    if(this->m_parent)
    {
        osg::ref_ptr<osg::MatrixTransform> lv_parentTransform = this->m_parent->getWorldTransform();
        osg::Matrix lv_worldTransformMatrix = lv_parentTransform->getMatrix() * this->getLocalTransform()->getMatrix();
        return new osg::MatrixTransform(lv_worldTransformMatrix);
    }
    else
        return this->m_localTransform;
}

void Entity::setParent(Entity* p_parent)
{
    this->m_parent = p_parent;
}

osg::ref_ptr<osg::MatrixTransform> Entity::getLocalTransform()
{
    return this->m_localTransform;
}

void Entity::move(moveOption p_moveOption, osg::Matrix p_transform)
{
    switch (p_moveOption)
    {
    case LOCAL_REL:
        this->m_localTransform->postMult(p_transform);
        break;

    case LOCAL_ABS:
        this->m_localTransform->setMatrix(p_transform);
        break;

    case WORLD_REL:
        if(this->m_parent)
            this->m_localTransform->postMult(this->m_parent->getWorldTransform()->getInverseMatrix() * p_transform);
        else
            this->m_localTransform->postMult(p_transform);
        break;

    case WORLD_ABS:
        if(this->m_parent)
            this->m_localTransform->setMatrix(this->m_parent->getWorldTransform()->getInverseMatrix() * p_transform);
        else
            this->m_localTransform->setMatrix(p_transform);
        break;
            
    default:
        break;
    }
}