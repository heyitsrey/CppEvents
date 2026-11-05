cmake_minimum_required(VERSION 3.15)
project(CppEvents VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Header-only library target
add_library(CppEvents INTERFACE)
target_include_directories(CppEvents INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(CppEvents INTERFACE cxx_std_17)

# Demo: Centralized EventBus
add_executable(demo_event_bus
        include/Event.hpp
        demo/demo_event_bus/main.cpp
        demo/demo_event_bus/EventBus.hpp
        demo/demo_event_bus/RailwaySystem.hpp
        demo/demo_event_bus/Logger.hpp
        demo/demo_event_bus/TicketSystem.hpp
        demo/demo_event_bus/DisplayBoard.hpp)
target_link_libraries(demo_event_bus PRIVATE CppEvents)

# Demo: Standalone Event
add_executable(demo_standalone
        include/Event.hpp
        demo/demo_standalone/main.cpp
        demo/demo_standalone/Alarm.hpp
        demo/demo_standalone/AlarmListener.hpp)
target_link_libraries(demo_standalone PRIVATE CppEvents)

# Demo: RAII ScopedConnection
add_executable(demo_scoped
        include/Event.hpp
        demo/demo_scoped/main.cpp)
target_link_libraries(demo_scoped PRIVATE CppEvents)

# Unit Tests (GoogleTest)
enable_testing()

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(test_event
        include/Event.hpp
        tests/test_event.cpp)
target_link_libraries(test_event PRIVATE CppEvents GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(test_event)
