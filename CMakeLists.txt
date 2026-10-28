cmake_minimum_required(VERSION 3.25)

project(river-control-center VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)
find_package(KF6Kirigami 6.0 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(WaylandClient REQUIRED IMPORTED_TARGET wayland-client)
find_program(WAYLAND_SCANNER_EXECUTABLE wayland-scanner REQUIRED)

set(FOREIGN_TOPLEVEL_XML
    ${CMAKE_CURRENT_SOURCE_DIR}/protocol/wlr-foreign-toplevel-management-unstable-v1.xml)
set(FOREIGN_TOPLEVEL_HEADER
    ${CMAKE_CURRENT_BINARY_DIR}/wlr-foreign-toplevel-management-unstable-v1-client-protocol.h)
set(FOREIGN_TOPLEVEL_CODE
    ${CMAKE_CURRENT_BINARY_DIR}/wlr-foreign-toplevel-management-unstable-v1-protocol.c)

add_custom_command(
    OUTPUT ${FOREIGN_TOPLEVEL_HEADER} ${FOREIGN_TOPLEVEL_CODE}
    COMMAND ${WAYLAND_SCANNER_EXECUTABLE} client-header
            ${FOREIGN_TOPLEVEL_XML} ${FOREIGN_TOPLEVEL_HEADER}
    COMMAND ${WAYLAND_SCANNER_EXECUTABLE} private-code
            ${FOREIGN_TOPLEVEL_XML} ${FOREIGN_TOPLEVEL_CODE}
    DEPENDS ${FOREIGN_TOPLEVEL_XML}
    VERBATIM
)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(river-control-center
    src/main.cpp
    src/keybindingscontroller.cpp
    src/keybindingscontroller.h
    src/settingscontroller.cpp
    src/settingscontroller.h
    src/windowenumerator.cpp
    src/windowenumerator.h
    ${FOREIGN_TOPLEVEL_CODE}
)

target_include_directories(river-control-center PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

qt_add_qml_module(river-control-center
    URI org.kaveen.rivercontrol
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/AppearancePage.qml
        qml/WindowsPage.qml
        qml/BarPanelsPage.qml
        qml/PowerIdlePage.qml
        qml/InputPage.qml
        qml/KeybindingsPage.qml
        qml/SessionPage.qml
        qml/AboutPage.qml
        qml/ColorSettingRow.qml
        qml/SettingCard.qml
)

configure_file(
    data/river-control-center.desktop.in
    ${CMAKE_CURRENT_BINARY_DIR}/river-control-center.desktop
    @ONLY
)

target_link_libraries(river-control-center PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    KF6::Kirigami
    PkgConfig::WaylandClient
)

install(TARGETS river-control-center RUNTIME DESTINATION bin)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/river-control-center.desktop
        DESTINATION share/applications)
