#pragma once

#include <vector>
#include <string>

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>
#include <implot_internal.h>

enum GraphType
{
    Force = 0,

    Q = 10,
    Q_deriv = 11,
    Q_deriv2 = 12,

    XYZ = 20,
    XYZ_deriv = 21,
    XYZ_deriv2 = 22,

    ABC_angles = 30,    
    ABC_deriv = 31,
    ABC_deriv2 = 32
};

class Graph
{
private:
    
public:
    double* m_current_value;
    std::string m_label;
    std::vector<double> m_data;
    
    Graph(std::string  p_label, double* p_current_value): m_current_value(p_current_value), m_label(p_label){}

    void update();

    void eraseData();
};

class Oscilloscope
{
private:
    std::string m_label;
    std::vector<Graph*> m_plotList;
    std::vector<Graph*> m_graphList;
    std::vector<double> m_x_data;
    int m_x_size;
    bool isXAxisInit = false;

    std::vector<double> fillZeros(std::vector<double> p_vector, int p_size);
public:
    Oscilloscope(std::string  p_label): m_label(p_label) {}

    void initXAxis(std::vector<double> p_x_data);
    void addPlot(Graph* p_plot);
    void show();
    void update();
    void eraseData();
};