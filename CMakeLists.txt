cmake_minimum_required(VERSION 3.21)
project(Link2BOM VERSION 0.1 LANGUAGES CXX)

set(APP_NAME Link2BOM)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(LINK2BOM_STRIP_BINARY "Strip symbols for smaller binaries" ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick QuickControls2)
qt_standard_project_setup(REQUIRES 6.5)

set(APP_SOURCES
    src/main.cpp
    src/app_icon.rc
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

set(APP_QML_FILES
    src/qml/Main.qml
    src/qml/components/SidebarPane.qml
    src/qml/components/SidebarModuleCard.qml
    src/qml/components/BomPane.qml
)

set(APP_RESOURCES
    assets/app.ico
    assets/Audiowide-Regular.ttf
    assets/Github-dark.png
    assets/Github-light.png
    assets/icon_100.png
)

qt_add_executable(${APP_NAME} ${APP_SOURCES})

set_target_properties(${APP_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
)

qt_add_resources(${APP_NAME} app_runtime_assets
    PREFIX "/"
    FILES
        assets/icon_100.png
)

qt_add_qml_module(${APP_NAME}
    URI ${APP_NAME}
    VERSION 1.0
    QML_FILES ${APP_QML_FILES}
    RESOURCES ${APP_RESOURCES}
)

target_link_libraries(${APP_NAME} PRIVATE Qt6::Quick Qt6::QuickControls2)

if (MINGW AND LINK2BOM_STRIP_BINARY)
    target_link_options(${APP_NAME} PRIVATE
        $<$<CONFIG:Release>:-s>
        $<$<CONFIG:MinSizeRel>:-s>
    )
endif()

install(TARGETS ${APP_NAME}
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)

