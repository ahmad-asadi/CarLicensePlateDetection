cmake_minimum_required(VERSION 3.7)
project(CarLicensePlateDetection)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp LPRegistrar.cpp LPRegistrar.h)

#include_directories(${OpenCV_LIBS})
find_package( OpenCV REQUIRED )

find_library( NETSDK ./lib/Linux\(20170519\)/x64/libxmnetsdk.so )

add_executable(CarLicensePlateDetection ${SOURCE_FILES})

target_link_libraries(CarLicensePlateDetection ${NETSDK} ${OpenCV_LIBS})
