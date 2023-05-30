cmake_minimum_required(VERSION 3.23)
project(Grafica)

set(CMAKE_CXX_STANDARD 14)

if (${APPLE})
    include_directories(/opt/homebrew/include)
    include_directories(/opt/homebrew/Cellar/freeglut/3.4.0/include)
endif ()

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/src)

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})
link_libraries(${OPENGL_LIBRARIES})

if (${APPLE})
    link_directories(/opt/local/lib)
    link_directories(/opt/homebrew/lib)
    link_directories(/opt/homebrew/Cellar/freeglut/3.4.0/lib)
endif ()

file(GLOB APP_SRC
        "*/*.h"
        "*/*.cpp"
        "*/*.c"
        )

add_executable(Grafica main.cpp ${APP_SRC})
target_link_libraries(Grafica glfw glew glut dl SOIL)

add_custom_command(TARGET Grafica POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/TEXTURES $<TARGET_FILE_DIR:Grafica>/TEXTURES)
