cmake_minimum_required(VERSION 3.16)
project(intel_msr_suite)
set(CMAKE_CXX_STANDARD 20)

# ============================================================================
# Shared library for common MSR code
# ============================================================================
add_library(msr_common STATIC
        src/msr/MsrOps.cpp
        src/msr/OcMailbox.cpp
)

# ============================================================================
# CLI Tool
# ============================================================================
add_executable(intel_msr_tool
        src/msr/main.cpp
        src/msr/CLI.cpp
)

target_link_libraries(intel_msr_tool PRIVATE msr_common)


# ============================================================================
# GUI Application
# ============================================================================
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/ui)

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)
find_package(KF6StatusNotifierItem REQUIRED)

add_executable(intel_msr_gui
        src/main.cpp
        src/MainWindow.h
        src/MainWindow.cpp
        ui/MainWindow.ui
        ui/resource.qrc
)

target_link_libraries(intel_msr_gui PRIVATE
        msr_common
        Qt::Core
        Qt::Gui
        Qt::Widgets
        KF6::StatusNotifierItem
)

target_compile_definitions(intel_msr_gui PRIVATE
        "$<$<CONFIG:DEBUG>:DEBUG>"
)
