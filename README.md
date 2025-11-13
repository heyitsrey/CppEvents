# CppEvents

A lightweight, type-safe event system for C++, inspired by C# events.  
Supports both single events and a centralized EventBus for multiple publishers and subscribers.
Ideal for small to medium projects or as a template for your own event system.
Requires C++17 or later.

## Features

- Subscriber/Publisher system (optional EventBus)
- Subscribe member functions directly (`+= std::pair(this, &Class::Method)`)
- Supports lambdas and free functions as listeners
- Multiple events and multiple publishers/subscribers
- Header-only, easy to include in any project

## Build example (C++17)

g++ -std=c++17 demo_event_bus/main.cpp -o demo_bus -Iinclude
g++ -std=c++17 demo_standalone/main.cpp -o demo_standalone -Iinclude

## License

MIT License