# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

StateSnap is a plugin module for Orbiter Space Flight Simulator that provides automatic periodic scenario snapshots (autosave functionality). It saves scenarios at configurable intervals based on real-world time, organizing saves into dated folders.

## Build System

This is a Windows C++ DLL project using CMake and the Orbiter SDK.

### Building with Orbiter source tree
```bash
# From Orbiter root, add this project to Extern/ and include in CMakeLists.txt
cmake -B build -S .
cmake --build build --config Release
```

### Building standalone
```bash
cmake -B build -S . -DSTATESNAP_STANDALONE=ON -DORBITER_SDK_DIR=<path-to-orbiter-sdk>
cmake --build build --config Release
```

Output: `build/Modules/Plugin/StateSnap.dll`

### Dependencies
- Orbiter SDK headers (`Orbitersdk.h`)
- Windows API and common controls (`commctrl.h`)
- Resource file `Resources.rc` defines the control dialog

## Architecture

### Core Components

**StateSnap class** (`StateSnap.h`, `StateSnap.cpp`): Main module class inheriting from `oapi::Module`
- Manages autosave timing using `std::chrono::steady_clock` (real-world time, not sim time)
- Handles configuration persistence via Orbiter's file API (`StateSnap.cfg`)
- Provides on-screen notifications using Orbiter's annotation system
- Key callbacks: `clbkSimulationStart`, `clbkSimulationEnd`, `clbkPostStep`

**Dialog system** (`StateSnapDlg.h`, `StateSnapDlg.cpp`): Win32 dialog for user control
- Accessed via Ctrl+F4 custom command menu in Orbiter
- Shows countdown timer, pause/resume, save now, interval adjustment (1-60 min)
- Uses a 1-second WM_TIMER for live countdown updates

**Resources** (`resource.h`, `Resources.rc`): Dialog layout and string table

### Save File Naming

Saves go to: `Scenarios/Autosave/<SystemDate>/<VesselName>-<SimDateTime>`
- System date (folder): Real-world date when save occurred
- Sim date/time: Converted from MJD using astronomical algorithm

### Key Orbiter SDK Functions Used

- `oapiSaveScenario()` - saves current state
- `oapiRegisterCustomCmd()` - adds menu entry
- `oapiOpenDialogEx()` / `oapiCloseDialog()` - dialog management
- `oapiCreateAnnotation()` - on-screen text notifications
- `oapiOpenFile()` / `oapiReadItem_*` / `oapiWriteItem_*` - config file I/O
- `oapiGetSimMJD()` - get simulation time as Modified Julian Date
- `oapiGetFocusObject()` / `oapiGetObjectName()` - get current vessel name
