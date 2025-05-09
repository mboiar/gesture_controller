[![CMake](https://github.com/mboiar/gesture_controller/actions/workflows/cmake.yml/badge.svg?branch=main)](https://github.com/mboiar/gesture_controller/actions/workflows/cmake.yml)

# Gesture controller

Modular, versatile gesture-based controller written in C++.

## Demo

[![Watch the video](https://img.youtube.com/vi/OrVqN6P2TyY/hqdefault.jpg)](https://youtu.be/OrVqN6P2TyY)

## Features
- Easily remappable gesture commands
- Connect to any device over serial supporting camera video stream
- Good performance on resource constrained hardware
- Face detection with the HaarCascade model 
- Gesture detection with the ResNet18 model trained on a custom dataset

## Usage
Compile project with `cmake --build .`
Run tests with `bin/gesture_controller__test`
Run app with `bin/gesture_controller`
