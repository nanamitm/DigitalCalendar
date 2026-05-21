cmake_minimum_required(VERSION 3.21)

project(DigitalCalendar VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_add_executable(DigitalCalendar
    desktop/src/main.cpp
    desktop/src/MainWindow.cpp
    desktop/src/MainWindow.h
    desktop/src/CalendarWidget.cpp
    desktop/src/CalendarWidget.h
    desktop/src/AppIcon.h
)

target_link_libraries(DigitalCalendar PRIVATE Qt6::Widgets)

set_target_properties(DigitalCalendar PROPERTIES
    WIN32_EXECUTABLE TRUE
)
