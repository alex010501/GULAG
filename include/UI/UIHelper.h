#pragma once

#include <stb_image.h>

#include <string>

// GUI include
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

// OSG include
#include <osgViewer/Viewer>
#include <osgViewer/config/SingleWindow>
#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgGA/TrackballManipulator>

namespace UIHelper
{

    struct ImageData
    {
        ImTextureID texture;
        int width;
        int height;
    };
    
    bool LoadTextureFromMemory(const void* data, size_t data_size, GLuint* out_texture, int* out_width, int* out_height);

    bool LoadTextureFromFile(const char* file_name, GLuint* out_texture, int* out_width, int* out_height);

    ImageData LoadImg(const char* filename);

    bool ImGui_imageButton(ImageData imageData, bool enabled, std::string buttonName);

    void ImGui_picture(ImageData imageData);
};