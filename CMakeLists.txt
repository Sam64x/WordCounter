cmake_minimum_required(VERSION 3.16)

project(WordCounters VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick QuickControls2)

qt_standard_project_setup(REQUIRES 6.5)

set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/application/file.rc")
qt_add_executable(appWordCounters
    src/main.cpp
    ${app_icon_resource_windows}
)

qt_add_qml_module(appWordCounters
    URI WordCounters
    VERSION 1.0
    QML_FILES
        qml/Main.qml
    SOURCES
        src/models/wordfrequencymodel.h
        src/models/wordfrequencymodel.cpp
        src/workers/wordcountworker.h
        src/workers/wordcountworker.cpp
        src/controllers/controller.h
        src/controllers/controller.cpp
    RESOURCES
        application.qrc
        parse_data.qrc
        qml.qrc
)

set_target_properties(appWordCounters PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_include_directories(appWordCounters
  PUBLIC
    ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(appWordCounters
    PRIVATE Qt6::Quick
    PRIVATE Qt6::QuickControls2
)

include(GNUInstallDirs)
install(TARGETS appWordCounters
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
