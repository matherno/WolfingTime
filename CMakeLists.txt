cmake_minimum_required(VERSION 3.9)
project(WolfingTime)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES src/main.cpp src/ByteWriter.cpp src/ByteWriter.h src/WolfRenderer.cpp src/WolfRenderer.h src/WolfMap.cpp src/WolfMap.h src/MapImporter.cpp src/MapImporter.h)
add_executable(WolfingTime ${SOURCE_FILES})

include_directories(./dependencies ./src)

set(LIB_MATHERNOGL /home/matt/SoftwareProjects/C++/WolfingTime/dependencies/libMathernoGL.a )
target_link_libraries(WolfingTime ${LIB_MATHERNOGL} GL GLEW glfw freeimage)