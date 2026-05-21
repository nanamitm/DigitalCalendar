cmake_minimum_required(VERSION 3.21)

project(DigitalCalendarAndroid VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_add_executable(DigitalCalendarAndroid
    src/main.cpp
    src/AndroidCalendarWidget.cpp
    src/AndroidCalendarWidget.h
)

target_link_libraries(DigitalCalendarAndroid PRIVATE Qt6::Widgets)

qt_import_plugins(DigitalCalendarAndroid
    INCLUDE_BY_TYPE
        platforms Qt6::QAndroidIntegrationPlugin
        styles Qt6::QAndroidStylePlugin
    EXCLUDE_BY_TYPE
        iconengines
        imageformats
        platforminputcontexts
)

set_target_properties(DigitalCalendarAndroid PROPERTIES
    QT_ANDROID_PACKAGE_NAME "org.codex.digitalcalendar"
    QT_ANDROID_APP_NAME "Digital Calendar"
    QT_ANDROID_VERSION_NAME "${PROJECT_VERSION}"
    QT_ANDROID_VERSION_CODE 1
    QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/package"
    QT_ANDROID_MIN_SDK_VERSION 28
    QT_ANDROID_TARGET_SDK_VERSION 35
    QT_ANDROID_COMPILE_SDK_VERSION 36
    QT_ANDROID_SDK_BUILD_TOOLS_REVISION 36.0.0
)
