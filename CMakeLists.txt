cmake_minimum_required(VERSION 3.20)

project(DicomViewer)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(VTK REQUIRED)
find_package(GDCM REQUIRED)

include_directories(${GDCM_INCLUDE_DIRS})

add_executable(DicomViewer
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/MainWindow.ui
)

message(STATUS "GDCM_LIBRARIES = ${GDCM_LIBRARIES}")
link_directories("C:/projects/Akgun/third_party/gdcm/install/lib")

target_link_libraries(DicomViewer
    PRIVATE
    Qt6::Widgets
    ${VTK_LIBRARIES}
    ${GDCM_LIBRARIES}
	gdcmMSFF
    gdcmCommon
    gdcmIOD
)

vtk_module_autoinit(
    TARGETS DicomViewer
    MODULES ${VTK_LIBRARIES}
)

add_custom_command(TARGET DicomViewer POST_BUILD
    COMMAND "${Qt6_DIR}/../../../bin/windeployqt.exe"
            "$<TARGET_FILE:DicomViewer>"
    COMMENT "Deploying Qt DLLs"
)