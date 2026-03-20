cmake_minimum_required(VERSION 3.21)
project(Link2BOM VERSION 1.0.0 LANGUAGES CXX)

set(APP_NAME Link2BOM)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTORCC ON)

option(LINK2BOM_STRIP_BINARY "Strip symbols for smaller binaries" ON)

set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_PIC ON CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_EXAMPLE_HO OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_BENCH OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_WARNINGS OFF CACHE BOOL "" FORCE)
set(SPDLOG_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(3rd-party/spdlog-1.17.0 EXCLUDE_FROM_ALL)
if (TARGET spdlog)
    set_target_properties(spdlog PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
endif()

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Quick QuickControls2 Charts Widgets)
qt_standard_project_setup(REQUIRES 6.5)
qt_policy(SET QTP0004 NEW)

set(APP_CPP_SOURCES
    src/main.cpp
    src/app/AppLogger.cpp
    src/app/AppController.cpp
    src/app/UiSettingsStore.cpp
    src/app/DefaultDataSeeder.cpp
    src/app/StatusHub.cpp
    src/app/SpreadsheetConverter.cpp
    src/app/CsvParsers.cpp
    src/app/DataIoController.cpp
    src/app/ThemeController.cpp
    src/app/ProjectController.cpp
    src/app/CategoryController.cpp
    src/app/BomTableModel.cpp
    src/app/ImportService.cpp
    src/app/ArchiveController.cpp
)

set(APP_HEADERS
    src/app/AppLogger.h
    src/app/AppController.h
    src/app/UiSettingsStore.h
    src/app/DefaultDataSeeder.h
    src/app/StatusHub.h
    src/app/SpreadsheetConverter.h
    src/app/CsvParsers.h
    src/app/ImportTypes.h
    src/app/DataIoController.h
    src/app/ThemeController.h
    src/app/ProjectController.h
    src/app/CategoryController.h
    src/app/BomTableModel.h
    src/app/ImportService.h
    src/app/ArchiveController.h
)

set(APP_PLATFORM_RESOURCES
    src/app_icon.rc
)

set(APP_QRC_FILES
    resources/assets.qrc
)

set(APP_QML_FILES
    src/qml/Main.qml
    src/qml/I18n.js
    src/qml/components/AppButton.qml
    src/qml/components/AppToolButton.qml
    src/qml/components/DebugConsole.qml
    src/qml/components/ModalShell.qml
    src/qml/components/DialogFooter.qml
    src/qml/components/FormField.qml
    src/qml/components/DialogHost.qml
    src/qml/components/DiffPane.qml
    src/qml/components/SidebarPane.qml
    src/qml/components/SidebarModuleCard.qml
    src/qml/components/SidebarProjectActions.qml
    src/qml/components/SidebarCategoryActions.qml
    src/qml/components/SidebarFilterTree.qml
    src/qml/components/TopBar.qml
    src/qml/components/BomPane.qml
)

qt_add_executable(${APP_NAME}
    ${APP_CPP_SOURCES}
    ${APP_HEADERS}
    ${APP_PLATFORM_RESOURCES}
    ${APP_QRC_FILES}
)

set_target_properties(${APP_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
)

qt_add_qml_module(${APP_NAME}
    URI ${APP_NAME}
    VERSION 1.0
    NO_CACHEGEN
    QML_FILES ${APP_QML_FILES}
    RESOURCES src/qml/components/qmldir
)

target_link_libraries(${APP_NAME} PRIVATE Qt6::Quick Qt6::QuickControls2 Qt6::Charts Qt6::Widgets spdlog::spdlog)

if (WIN32)
    target_link_libraries(${APP_NAME} PRIVATE dwmapi)
endif()

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




