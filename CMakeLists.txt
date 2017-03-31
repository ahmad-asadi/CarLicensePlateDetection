cmake_minimum_required(VERSION 3.7)
project(CarLicensePlateDetection)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp)

#include_directories(${OpenCV_LIBS})
find_package( OpenCV REQUIRED )

add_executable(CarLicensePlateDetection ${SOURCE_FILES})

target_link_libraries(CarLicensePlateDetection ${OpenCV_LIBS})
