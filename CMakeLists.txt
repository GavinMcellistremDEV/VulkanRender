cmake_minimum_required (VERSION 3.11)

project ("render")

add_executable(out RenderBase.h RenderBase.cpp main.cpp)
add_subdirectory(SDL EXCLUDE_FROM_ALL)
add_subdirectory(volk)
add_subdirectory(glm)
target_include_directories(out PRIVATE ${CMAKE_SOURCE_DIR}/SDL/include/SDL3)
target_link_libraries(out PRIVATE glm::glm)
target_link_libraries(out PRIVATE SDL3::SDL3-shared)
target_link_libraries(out PRIVATE volk_headers)


