#
#   This file is part of Magnum.
#
#   Original authors — credit is appreciated but not required:
#
#       2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019,
#       2020, 2021, 2022, 2023 — Vladimír Vondruš <mosra@centrum.cz>
#       2020 — Nghia Truong <nghiatruong.vn@gmail.com>
#
#   This is free and unencumbered software released into the public domain.
#
#   Anyone is free to copy, modify, publish, use, compile, sell, or distribute
#   this software, either in source code form or as a compiled binary, for any
#   purpose, commercial or non-commercial, and by any means.
#
#   In jurisdictions that recognize copyright laws, the author or authors of
#   this software dedicate any and all copyright interest in the software to
#   the public domain. We make this dedication for the benefit of the public
#   at large and to the detriment of our heirs and successors. We intend this
#   dedication to be an overt act of relinquishment in perpetuity of all
#   present and future rights to this software under copyright law.
#
#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#   THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
#   IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
#   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

cmake_minimum_required(VERSION 3.5)

find_package(Corrade REQUIRED
    Main
    Utility)

find_package(Magnum REQUIRED
    DebugTools
    GL
    MeshTools
    Primitives
    Shaders
    Trade
    Sdl2Application)

set_directory_properties(PROPERTIES CORRADE_USE_PEDANTIC_FLAGS ON)


#   Name of .exe file = magnum-octree 
#add_executable(magnum-octree WIN32
#    ./octree/LooseOctree.h
#    ./octree/LooseOctree.cpp
#    Octree.cpp
#    ./arcball/ArcBall.cpp)

#   !!! add_executable(... WIN32 ...) == NO CONSOLE
add_executable(RMD 
    RMD.cpp
    ./octree/LooseOctree.h
    ./octree/LooseOctree.cpp
    ./arcball/Arcball.cpp)

target_link_libraries(RMD PRIVATE
    Corrade::Main
    Corrade::Utility
    Magnum::Application
    Magnum::DebugTools
    Magnum::GL
    Magnum::Magnum
    Magnum::MeshTools
    Magnum::Primitives
    Magnum::Shaders
    Magnum::Trade)

install(TARGETS RMD DESTINATION ${MAGNUM_BINARY_INSTALL_DIR})

# Make the executable a default target to build & run in Visual Studio
set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT RMD)