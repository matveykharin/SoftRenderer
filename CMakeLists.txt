cmake_minimum_required(VERSION 3.10)
project(SoftRenderer CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(SoftRenderer
    main.cpp model.cpp tgaimage.cpp camera.cpp rasterizer.cpp
    geometry.h model.h tgaimage.h camera.h rasterizer.h shader.h shaders.h)
