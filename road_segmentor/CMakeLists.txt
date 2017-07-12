cmake_minimum_required (VERSION 2.8)

PROJECT(RoadConnectionLabelling)

FIND_PACKAGE( OpenCV REQUIRED )

# Requires OpenCV v2.4.1 or later
IF (${OpenCV_VERSION} VERSION_LESS 2.4.1)
    MESSAGE(FATAL_ERROR "OpenCV version is not compatible : ${OpenCV_VERSION}. Requires atleast OpenCV v2.4.1")
ENDIF()


SET (SRC
    ${CMAKE_BINARY_DIR}/src/main.cpp
    ${CMAKE_BINARY_DIR}/src/segmentation_methods.cpp
    ${CMAKE_BINARY_DIR}/src/thinning_methods.cpp
    ${CMAKE_BINARY_DIR}/src/flood_fill_methods.cpp
    ${CMAKE_BINARY_DIR}/src/json.hpp
    ${CMAKE_BINARY_DIR}/src/road_segmentation.h
)
SET (CMAKE_CXX_FLAGS "-std=c++11")
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
ADD_EXECUTABLE( ${PROJECT_NAME} ${SRC} )
TARGET_LINK_LIBRARIES( ${PROJECT_NAME}  ${OpenCV_LIBS} )
