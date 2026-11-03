cmake_minimum_required(VERSION 3.22)
project(mcbe_esp CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dobby hook framework
include(FetchContent)
FetchContent_Declare(
    Dobby
    GIT_REPOSITORY https://github.com/jmpews/Dobby.git
    GIT_TAG        v2.0
)
set(DOBBY_DEBUG OFF CACHE BOOL "" FORCE)
set(Plugin.SymbolResolver OFF CACHE BOOL "" FORCE)
set(Plugin.ImportTableReplace OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(Dobby)

set(SRC
    app/src/main/cpp/main.cpp
    app/src/main/cpp/hook/hooks.cpp
    app/src/main/cpp/esp/esp.cpp
    app/src/main/cpp/memory/memory.cpp
    app/src/main/cpp/render/overlay.cpp
)

add_library(mcbe_esp SHARED ${SRC})

target_include_directories(mcbe_esp PRIVATE
    app/src/main/cpp
)

target_link_libraries(mcbe_esp
    dobby
    android
    log
    EGL
    GLESv2
)

# arm64-v8a only — set in local.properties or gradle:
# abiFilters "arm64-v8a"
