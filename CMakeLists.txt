cmake_minimum_required(VERSION 3.10)
project(ForgeFiberSensors)

set(CMAKE_CXX_STANDARD 14)

add_executable(sensor_system
  src/main.cpp
  src/sensor_manager.cpp
  src/data_processor.cpp
)
