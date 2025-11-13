cmake_minimum_required(VERSION 4.0)
project(CppEvents)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(demo_event_bus
        include/Event.hpp
        demo_event_bus/main.cpp
        demo_event_bus/EventBus.hpp
        demo_event_bus/RailwaySystem.hpp
        demo_event_bus/Logger.hpp
        demo_event_bus/TicketSystem.hpp
        demo_event_bus/DisplayBoard.hpp)

add_executable(demo_standalone
        include/Event.hpp
        demo_standalone/main.cpp
        demo_standalone/Alarm.hpp
        demo_standalone/AlarmListener.hpp)
