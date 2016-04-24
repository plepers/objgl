cmake_minimum_required (VERSION 2.6)


project (objgl)


set( PROJECT_SOURCES

  obj.cpp
  collapser.cpp
  main.cpp

  obj.h
  collapser.h
  main.h

)

include_directories("${PROJECT_SOURCE_DIR}/submodules/tclap/include")
include_directories("${PROJECT_SOURCE_DIR}/submodules/tinyobjloader")
include_directories("${PROJECT_SOURCE_DIR}/submodules/glm")

add_executable(objgl ${PROJECT_SOURCES})