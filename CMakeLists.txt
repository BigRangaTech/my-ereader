cmake_minimum_required(VERSION 3.21)

project(MyEreader LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

option(ENABLE_TTS "Enable TextToSpeech" ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick Qml Network Sql)
if(ENABLE_TTS)
  find_package(Qt6 COMPONENTS TextToSpeech)
endif()

add_subdirectory(src/core)
add_subdirectory(src/crypto)
add_subdirectory(src/formats)
add_subdirectory(src/sync)
add_subdirectory(src/tts)
add_subdirectory(src/app)
