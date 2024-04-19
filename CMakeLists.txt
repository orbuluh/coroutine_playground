cmake_minimum_required(VERSION 3.14...3.28)

set(CMAKE_CXX_STANDARD 20)

project(
  ModernCMakeExample
  VERSION 0.1
  DESCRIPTION "Coroutine Playground"
  LANGUAGES CXX)

add_subdirectory(util)
add_subdirectory(src)
