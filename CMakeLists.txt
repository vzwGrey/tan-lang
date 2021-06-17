cmake_minimum_required(VERSION 3.7...3.20)
if(${CMAKE_VERSION} VERSION_LESS 3.12)
  cmake_policy(VERSION ${CMAKE_VERSION_MAJOR}.${CMAKE_VERSION_MINOR})
endif()

project(tan VERSION 0.1.0)

add_subdirectory(src)
