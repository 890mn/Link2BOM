cmake_minimum_required(VERSION 3.21)
project(Link2BOM VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick QuickControls2)
qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(Link2BOM
    src/main.cpp
    src/app/AppController.cpp
    src/app/AppController.h
    src/app/ThemeController.cpp
    src/app/ThemeController.h
    src/app/ProjectController.cpp
    src/app/ProjectController.h
    src/app/CategoryController.cpp
    src/app/CategoryController.h
    src/app/BomTableModel.cpp
    src/app/BomTableModel.h
    src/app/ImportService.cpp
    src/app/ImportService.h
)

qt_add_qml_module(Link2BOM
    URI Link2BOM
    VERSION 1.0
    QML_FILES
        src/qml/Main.qml
        src/qml/components/SidebarPane.qml
        src/qml/components/SidebarModuleCard.qml
        src/qml/components/BomPane.qml
    RESOURCES
        src/asset/Audiowide-Regular.ttf
        src/asset/Github-dark.png
        src/asset/Github-light.png
        src/asset/icon_100.png
)

target_link_libraries(Link2BOM PRIVATE Qt6::Quick Qt6::QuickControls2)

install(TARGETS Link2BOM
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)
