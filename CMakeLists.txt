cmake_minimum_required(VERSION 3.25)
project(KEYFORGE VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT WIN32)
  message(FATAL_ERROR "KEYFORGE targets Windows 11 only.")
endif()

add_executable(KEYFORGE WIN32
  src/main.cpp
  src/App.cpp
  src/App.h
  src/AudioEngine.cpp
  src/AudioEngine.h
  src/KeyboardEngine.cpp
  src/KeyboardEngine.h
  src/Config.cpp
  src/Config.h
  src/Wav.cpp
  src/Wav.h
  src/Logger.cpp
  src/Logger.h
  src/Resource.h
  src/KEYFORGE.rc
)

target_compile_definitions(KEYFORGE PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)
target_compile_options(KEYFORGE PRIVATE /W4 /permissive- /EHsc)
target_link_libraries(KEYFORGE PRIVATE user32 shell32 ole32 avrt xaudio2)

if(MSVC)
  set_property(TARGET KEYFORGE PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

install(TARGETS KEYFORGE RUNTIME DESTINATION .)
install(DIRECTORY sounds/ DESTINATION sounds)
install(FILES README.md SECURITY.md ARCHITECTURE.md CONTRIBUTING.md CHANGELOG.md DESTINATION .)
