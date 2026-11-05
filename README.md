# CppEvents

A lightweight, thread-safe, header-only event system for C++17, inspired by C# events.  
Supports both standalone events and a centralized `EventBus` pattern for multiple publishers and subscribers.

## Features

- **Header-only & Zero Dependencies:** Simply include [`include/Event.hpp`](include/Event.hpp) in your project.
- **C#-Style Syntax:** Subscribe with `+=` and unsubscribe with `-=`.
- **Flexible Listeners:**
  - Lambdas, free functions, and `std::function`
  - Member functions (`const` and non-`const`) via `std::pair(this, &Class::method)`
  - Smart pointers (`std::weak_ptr` and `std::shared_ptr`) via `std::pair(ptr, &Class::method)`
- **RAII Lifetime Management:** Optional `ScopedConnection` via `subscribeScoped()` automatically disconnects when destroyed (safe regardless of destruction order).
- **Thread-Safe & Re-entrant:** Protected by internal synchronization; listeners can safely subscribe or unsubscribe during event notification without deadlocks or iterator invalidation.

## Quick Start

See [`demo/demo_event_bus`](demo/demo_event_bus), [`demo/demo_standalone`](demo/demo_standalone), and [`demo/demo_scoped`](demo/demo_scoped) for complete working examples.

## Building the Demos & Running Tests

### Using CMake & CTest

```bash
cmake -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

### Using `g++` Directly (C++17)

```bash
g++ -std=c++17 -Iinclude demo/demo_event_bus/main.cpp -o demo_bus
g++ -std=c++17 -Iinclude demo/demo_standalone/main.cpp -o demo_standalone
g++ -std=c++17 -Iinclude demo/demo_scoped/main.cpp -o demo_scoped
```

## License

MIT License
