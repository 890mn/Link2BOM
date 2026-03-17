# Link2BOM

Version: 1.0.0 (2026-03-13)

English | [中文](README.zh.md)

## Overview

Link2BOM is a Qt 6 / Qt Quick desktop tool for importing, organizing, filtering, comparing, and exporting BOM data.
This is the first stable release focused on a reliable daily workflow.

## Feature Highlights (1.0.0)

- Split LCSC vs generic spreadsheet import with robust CSV handling
- Project, category, filter, and search workflows for BOM management
- Diff analysis with charts (Qt Charts)
- Local archive slots with save/load and overwrite confirmation
- CSV export and UI theme/language switching

## Build (Windows + MinGW)

Prerequisites:
- Qt 6.10.1 (mingw_64)
- CMake 3.21+
- MinGW 13.1 (from Qt Tools)
- Python (optional, for `.xls` conversion)

Build:
```powershell
cmake --build ../build --config Debug --target Link2BOM -j4
```

Run:
```powershell
.\build\Link2BOM.exe
```

## Packaging (Portable Release)

Use the deploy script under `scripts/`:
```powershell
pwsh .\scripts\deploy_from_build.ps1 -BuildDir "..\build" -AppName "Link2BOM" -QtBin "C:/Qt/6.10.1/mingw_64/bin"
```

Notes:
- The script runs `windeployqt`, removes FluentWinUI3 theme, and compresses binaries with UPX
- Output: `build/dist` and `build/Link2BOM-windows-portable.zip`

## Data Notes

- LCSC import expects the LCSC export template and will not overwrite mismatched headers
- Generic import can replace headers when needed
- Local archives are stored under `AppData/Local/Link2BOM/saves`
