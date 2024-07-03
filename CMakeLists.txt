cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED)
option(HELLOIMGUI_USE_GLFW3 "Use Glfw3 as a platform backend" ON)
option(HELLOIMGUI_HAS_OPENGL3 "Use OpenGL3 as a rendering backend" ON)
project(TStudio VERSION 0.1.0 LANGUAGES C CXX)

add_executable(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/src/main.cpp)

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/assets
                ${CMAKE_CURRENT_BINARY_DIR}/assets)


# src subdirectories
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/src)
add_subdirectory( ${CMAKE_SOURCE_DIR}/src/core)
add_subdirectory( ${CMAKE_SOURCE_DIR}/src/hardware)
add_subdirectory( ${CMAKE_SOURCE_DIR}/src/library)
add_subdirectory( ${CMAKE_SOURCE_DIR}/src/ui)


# LabSound 
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/LabSound)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/lib/LabSound/include)
target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/bin/LabSound_d.framework/LabSound_d)
target_link_libraries(${PROJECT_NAME} PUBLIC ${CMAKE_BINARY_DIR}/bin/libLabSoundMiniAudio_d.a)
target_link_libraries(${PROJECT_NAME} PUBLIC ${CMAKE_BINARY_DIR}/bin/libLabSoundRtAudio_d.a)
target_link_libraries(${PROJECT_NAME} PUBLIC ${CMAKE_BINARY_DIR}/lib/LabSound/third_party/libnyquist/lib/liblibnyquist_d.a)

# choc
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/lib/choc)

# RTMidi
find_package(rtmidi CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE RtMidi::rtmidi)

#Brickworks
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/lib/brickworks/include)

# libusb
find_package(PkgConfig REQUIRED)
pkg_check_modules(libusb REQUIRED IMPORTED_TARGET libusb-1.0)
target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::libusb)

#cairo
find_package(PkgConfig REQUIRED)
pkg_check_modules(CAIROMM REQUIRED IMPORTED_TARGET cairomm-1.16)
target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::CAIROMM)

# glad
find_package(glad CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glad::glad)

# hello-imgui
find_package(hello-imgui CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE hello-imgui::hello_imgui)

# stduuid
find_package(stduuid CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE stduuid)

# yaml-cpp
find_package(yaml-cpp CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE yaml-cpp::yaml-cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

set(RESOURCE_FILES
  ${CMAKE_SOURCE_DIR}/assets/BVKER - The Astro Perc 08.wav
  ${CMAKE_SOURCE_DIR}/assets/BVKER - The Astro Perc 10.wav
  )



if(APPLE)
  target_link_libraries(${PROJECT_NAME} PRIVATE
  "-framework CoreMidi"
  "-framework Cocoa"
  "-framework Accelerate"
  "-framework AudioToolbox"
  "-framework AudioUnit"
  "-framework CoreServices"
  "-framework CoreAudio"
  "-framework Carbon"
  "-framework CoreFoundation"
    "-framework IOKit"
  )
endif()