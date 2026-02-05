cmake_minimum_required(VERSION 3.21)
project(StarBOM VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

qt_add_executable(StarBOM
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
)

target_link_libraries(StarBOM PRIVATE Qt6::Widgets)

install(TARGETS StarBOM
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)
