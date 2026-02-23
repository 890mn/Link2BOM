# Link2BOM

Link2BOM 是一个基于 Qt 6 + Qt Quick 的桌面 BOM 管理原型，用于导入 BOM 并进行项目化整理。

## 项目结构
- `src/app`：C++ 业务与模型层（控制器、表格模型、导入服务）。
- `src/qml`：界面层（`Main.qml` 与 `components/`）。
- `assets`：原始资源文件（字体、图标、图片）。
- `resources/assets.qrc`：资源入口（统一把大资源打进 Qt 资源系统）。
- `src/app_icon.rc`：Windows exe 图标入口。
- `scripts`：构建/部署脚本。

## 推荐构建与打包（Windows）
### 1) 最小体积构建（MinSizeRel）
### 2) 从 build 目录直接部署
> 该脚本会：
> - 复制 `build\Link2BOM.exe` 到 `build\dist`
> - 对 `build\dist\Link2BOM.exe` 执行 `windeployqt`
> - 生成 `build\Link2BOM-windows-portable.zip`

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\deploy_from_build.ps1
```
