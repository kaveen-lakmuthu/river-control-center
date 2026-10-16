cmake_minimum_required(VERSION 3.25)

project(river-control-center VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)
find_package(KF6Kirigami 6.0 REQUIRED)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(river-control-center
    src/main.cpp
    src/settingscontroller.cpp
    src/settingscontroller.h
)

qt_add_qml_module(river-control-center
    URI org.kaveen.rivercontrol
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/AppearancePage.qml
        qml/WindowsPage.qml
        qml/BarPanelsPage.qml
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
)

install(TARGETS river-control-center RUNTIME DESTINATION bin)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/river-control-center.desktop
        DESTINATION share/applications)
