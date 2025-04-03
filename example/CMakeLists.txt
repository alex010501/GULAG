# set sources for example
set(SOURCES main.cpp)

add_executable(example ${SOURCES})

target_link_libraries(example PUBLIC GUI_Core)

# target_compile_options(example PRIVATE $<$<CONFIG:Debug>:/MTd>)
# target_compile_options(example PRIVATE $<$<CONFIG:Release>:/MT>)