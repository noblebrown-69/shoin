cmake_minimum_required(VERSION 3.16)

project(Shoin VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets PrintSupport)

find_path(HUNSPELL_INCLUDE_DIR hunspell/hunspell.h)
find_library(HUNSPELL_LIBRARY hunspell-1.7)

include_directories(${HUNSPELL_INCLUDE_DIR})

qt_standard_project_setup()

qt_add_executable(Shoin
    main.cpp
    ShoinFrame.cpp
    MonasteryEditor.cpp
)

target_sources(Shoin PRIVATE resources.qrc)

target_link_libraries(Shoin PRIVATE Qt6::Core Qt6::Widgets Qt6::PrintSupport ${HUNSPELL_LIBRARY})

set_target_properties(Shoin PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)