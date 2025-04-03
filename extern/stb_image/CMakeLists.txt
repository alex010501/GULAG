cmake_minimum_required(VERSION 3.13)
project(STB_IMAGE)

add_library(stb_image STATIC include/stb_image.h stb_image_impl.c)

# target_compile_options(stb_image PUBLIC $<$<CONFIG:Debug>:/MTd>)
# target_compile_options(stb_image PUBLIC $<$<CONFIG:Release>:/MT>)

target_include_directories(stb_image PUBLIC include/)