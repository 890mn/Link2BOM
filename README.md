# StarBOM

StarBOM 是一个基于 Qt 6 Widgets 的桌面原型，用于导入 BOM 并进行半自动整理和项目化管理。

## 当前原型特性
- 左侧 20% 导航工作区（可滚动）：导入、导出、项目管理、分类组管理、主题切换。
- 右侧 80% 内容区：顶部 10% 视图切换 + 搜索框，底部 90% 主视图内容。
- 自适应窗口初始尺寸（按当前屏幕可用区域 90% 启动），并设置最小宽度避免全屏下侧栏控件被压缩。
- 提供三元色主题切换（Aurora / Citrus / Slate）。
- XLS/XLSX 导入流程已接入：优先使用内置 Python 解析 xlsx；其余情况尝试 libreoffice/ssconvert 转 CSV，再填充 BOM 视图。
- 导出先保留 CSV 通道。

## 工具链配置（容器/Ubuntu）
```bash
./scripts/setup_qt6_toolchain_ubuntu.sh
./scripts/verify_qt6_toolchain.sh
```

更多细节见 `docs/container-toolchain.md`。
