# Link2BOM

版本：1.0.0（2026-03-13）

[English](README.md) | 中文

## 项目简介

Link2BOM 是一个基于 Qt 6 / Qt Quick 的桌面工具，用于导入、整理、筛选、对比并导出 BOM 数据
当前为第一个稳定版，重点保证日常使用流程的稳定性

## 1.0.0 功能亮点

- 立创导入与通用表格导入分离，CSV 处理更稳定
- 项目/分类/筛选/搜索等 BOM 管理流程
- 差异分析与图表（Qt Charts）
- 本地存档槽位保存/读取，覆盖保存二次确认
- CSV 导出与主题/语言切换

## 自行编译（Windows + MinGW）

环境要求：
- Qt 6.10.1（mingw_64）
- CMake 3.21+
- MinGW 13.1（Qt Tools 内置）
- Python（可选，用于 `.xls` 转换）

编译：
```powershell
cmake --build ../build --config Debug --target Link2BOM -j4
```

运行：
```powershell
.\build\Link2BOM.exe
```

## 打包发行版（便携包）

使用 `scripts/` 下的脚本：
```powershell
pwsh .\scripts\deploy_from_build.ps1 -BuildDir "..\build" -AppName "Link2BOM" -QtBin "C:/Qt/6.10.1/mingw_64/bin"
```

说明：
- 脚本会执行 `windeployqt`，移除 FluentWinUI3 主题，并使用 UPX 压缩
- 输出位置：`build/dist` 与 `build/Link2BOM-windows-portable.zip`

## 数据说明

- 立创导入要求匹配立创导出模板，表头不一致时不会覆盖现有数据
- 通用导入在必要时可替换表头
- 本地存档默认路径：`AppData/Local/Link2BOM/saves`
