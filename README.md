# PrecizaGUI
A Testing GUI for Sucabot Preciza Tracker (Visteko Tracker)

## Overview
This project is a testing GUI for API of Sucabot Preciza Tracker 

## Prerequisites

Known good build dependecies:

- [CMake](https://cmake.org/download/)
- [Qt 5.10.1](https://www.qt.io/download)
- [VTK 7.1.1](https://github.com/Kitware/VTK/tree/v7.1.1)

Build pass on Windows 10 with MSVC 2015. Test on your own on other platforms and compilers.

## Installing
-CMAKE: choose PrecizaGUI folder for source code and fill in the corresponding qt and vtk directory after pressing "Configure"
-After Compiling, suggest to use windeployqt.exe in Qt to move Qt dll to program folder

## Deployment
-"Tracker is connected" is shown in the browser once Visteko tracker is detected.
-For "Start Tracking" is pressed, the corresponding "Tool" or "Point" is tracked.
-For "Tool",  NDI Polaris_Rigid_Body_(8700339) is detected. Position and Quaternion is shown.
-For "Point", detected stray marker position is shown.

## Authors
Allan Lee <allan.lee@sucabot.com>