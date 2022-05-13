cmake_minimum_required(VERSION 3.15)

project(getPM2D5 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    list(APPEND CMAKE_PREFIX_PATH "C:/Tools/OSGeo4W/apps/Qt5/lib/cmake/Qt5")
endif ()


find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(GDAL)


set(TS_FILES ./i18n/getPM2D5_zh_CN.ts)

add_executable(getPM2D5

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
target_include_directories(getPM2D5 PRIVATE ${GDAL_INCLUDE_DIR})

if (MSVC)
    set(qwt_lib "C:/Tools/OSGeo4W/apps/Qt5/lib/qwt.lib")
    INCLUDE_DIRECTORIES(C:/Tools/OSGeo4W/apps/Qt5/include/qwt6)
else ()
    set(qwt_lib "-lqwt")
    INCLUDE_DIRECTORIES(/usr/include/qwt)
endif ()
target_link_libraries(getPM2D5
        PRIVATE Qt5::Widgets
        PRIVATE ${GDAL_LIBRARY}
        ${qwt_lib})

if (MSVC)
    target_compile_options(getPM2D5 PRIVATE /utf-8)
endif ()
