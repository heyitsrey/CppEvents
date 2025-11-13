cmake_minimum_required(VERSION 4.0)
project(EventBusCPP)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(EventBusCPP
        demo/main.cpp
        include/Event.hpp
        demo/EventBus.hpp
        demo/RailwaySystem.hpp
        demo/Logger.hpp
        demo/TicketSystem.hpp
        demo/DisplayBoard.hpp)
