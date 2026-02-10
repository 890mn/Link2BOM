# Link2BOM

Link2BOM 是一个基于 Qt 6 + Qt Quick 的桌面 BOM 管理原型，用于导入 BOM 并进行项目化整理。

## 项目结构（优化后）
- `src/app`：C++ 业务与模型层（控制器、表格模型、导入服务）。
- `src/qml`：界面层（`Main.qml` 与 `components/`）。
- `assets`：字体、图标、图片等资源。
- `src/app_icon.rc`：Windows exe 图标入口。
- `scripts`：构建/部署脚本。

## 推荐构建与打包（Windows）
### 1) 最小体积构建（MinSizeRel）
```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build_minsizerel.ps1
```

### 2) 从 build 目录直接部署（你想要的流程）
> 该脚本会：
> - 复制 `build\Link2BOM.exe` 到 `build\dist`
> - 对 `build\dist\Link2BOM.exe` 执行 `windeployqt`
> - 生成 `build\Link2BOM-windows-portable.zip`

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\deploy_from_build.ps1
```

## 体积优化建议（为什么还可能到 100MB）
Qt Quick + QML 应用本身依赖较多，体积通常会比 Widgets 大。可继续尝试：
- 用 MinSizeRel + strip（本项目已在 MinGW 下默认开启 `-s`）。
- 在部署时裁剪不需要模块（如不需要 WebEngine、多媒体等）。
- 如果可接受，部署时去掉 QML import（`deploy_from_build.ps1 -NoQml`）用于验证最小集。
- 最终分发时使用 zip/7z 压缩（脚本已自动产出 zip）。

## 图标说明
- 应用窗口图标：`assets/icon_100.png`
- 资源管理器中 exe 图标：`assets/app.ico`（由 `src/app_icon.rc` 注入）
