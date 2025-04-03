#pragma once

#define M_PI 3.14159265358979323846

#include <cmath>

#include <UI/UIWindow.h>

#include <3D/Entity.h>

class UIWindow_PropertiesViewer: public UIWindow {
private:
    Entity* m_selectedEntity = nullptr;

    bool m_transformLocal = false;
    double m_translation[3] = {0.0, 0.0, 0.0};
    double m_rotation[3] = {0.0, 0.0, 0.0};
    bool m_isMoved = false;

    osg::Vec4 m_color;
    double m_transparency = 1.0;
    bool m_isColorChanged = false;

    void drawTransformInputs(double* p_translation, double* p_rotation);

    void getTransform();
    void setTransform();

    void getTranslationAndRotation(osg::Matrix p_matrix);
    osg::Matrix getMatrix();
    double degToRad(double p_deg) {return p_deg * (M_PI / 180.0);}
    double radToDeg(double p_rad) {return p_rad * (180.0 / M_PI);}

public:
    // Add variables here


    // Add class methods here
    UIWindow_PropertiesViewer(const char* p_title);
    void draw();
    void init();

    void setSelectedEntity(Entity* p_entity) {m_selectedEntity = p_entity;}

};