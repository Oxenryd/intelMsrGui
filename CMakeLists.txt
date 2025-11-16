cmake_minimum_required(VERSION 4.0)
project(intel_msr_gui)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS
        ${CMAKE_CURRENT_SOURCE_DIR}/ui
)

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

add_executable(intel_msr_gui
        src/main.cpp

        src/MainWindow.h
        src/MainWindow.cpp

        src/msr/IntelTypes.hpp
        src/msr/Msr.hpp
        src/msr/MsrOps.cpp
        src/msr/OcMailbox.cpp
        src/msr/Parameters.hpp

        ui/MainWindow.ui
        ui/resource.qrc
)

target_link_libraries(intel_msr_gui
        Qt::Core
        Qt::Gui
        Qt::Widgets
)

