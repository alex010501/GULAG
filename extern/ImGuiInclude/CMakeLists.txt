# CMake version
cmake_minimum_required(VERSION 3.19)

# Project name
project(imguiInclude)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(imguiInclude STATIC ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imconfig.h
                                ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.h
                                ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp
                                ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp
                                ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_internal.h
                                ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp
                                ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imstb_rectpack.h
                                ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imstb_textedit.h
                                ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imstb_truetype.h
                                ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp
                                ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_demo.cpp
                                ${CMAKE_CURRENT_SOURCE_DIR}/implot/implot.h
                                ${CMAKE_CURRENT_SOURCE_DIR}/implot/implot.cpp
                                ${CMAKE_CURRENT_SOURCE_DIR}/implot/implot_internal.h
                                ${CMAKE_CURRENT_SOURCE_DIR}/implot/implot_items.cpp
                                ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_glfw.h
                                ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_glfw.cpp
                                ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_opengl3.h
                                ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_opengl3.cpp)

# target_compile_options(imguiInclude PUBLIC $<$<CONFIG:Debug>:/MTd>)
# target_compile_options(imguiInclude PUBLIC $<$<CONFIG:Release>:/MT>)

target_link_libraries(imguiInclude PUBLIC glfw)
target_link_libraries(imguiInclude PUBLIC OpenGL::GL)
target_include_directories(imguiInclude PUBLIC ${Stb_INCLUDE_DIR})

target_include_directories(imguiInclude PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/imgui
                                               ${CMAKE_CURRENT_SOURCE_DIR}/implot)