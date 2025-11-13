# EventBusCPP

A lightweight, type-safe event system for C++.  
Supports member functions, lambdas, and events with arbitrary parameters.  
Ideal for small to medium projects or as a template for your own event system.

## Features

- Subscribe member functions directly (`+= std::pair(this, &Class::Method)`)
- Supports lambdas and free functions as listeners
- Multiple events and multiple publishers/subscribers
- Header-only, easy to include in any project

## Build example

g++ -std=c++17 demo/main.cpp -o demo
./demo