cmake_minimum_required(VERSION 3.10)

project(getPM2D5 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

INCLUDE_DIRECTORIES(/usr/include/qwt/)
find_package(Qt5 COMPONENTS Widgets LinguistTools REQUIRED)
find_package(GDAL)


set(TS_FILES ./i18n/getPM2D5_zh_CN.ts)

if (ANDROID)
    add_library(getPM2D5 SHARED
        main.cpp
        getpm2d5.cpp
        getpm2d5.ui
        singleprocess.cpp
        singleprocess.ui
        translation.qrc
        icons.qrc
        ${TS_FILES}
        )
else ()
    add_executable(getPM2D5 WIN32

        main.cpp
        getpm2d5.cpp
        getpm2d5.h
        getpm2d5.ui
        singleprocess.cpp
        singleprocess.h
        singleprocess.ui
        batchprocess.cpp
        batchprocess.h
        batchprocess.ui
        translation.qrc
        icons.qrc
        ${TS_FILES}
        )
endif ()
target_link_libraries(getPM2D5
    PRIVATE Qt5::Widgets
    PRIVATE ${GDAL_LIBRARY}
    -lqwt)

qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
