# 简单文本编辑器

Qt 实验二做的多窗口文本编辑器，界面用 Qt Designer 搭，主窗口之外还有关于、查找、替换三个对话框。

## 功能

- 文件：新建、打开、保存、另存为，关闭前检查未保存的修改
- 编辑：撤销、恢复、剪切、复制、粘贴、全选
- 格式：字体、字体颜色、文字底色、编辑器背景色、自动换行、显示行号
- 查找替换：向上/向下、区分大小写、到末尾后回到开头继续找、全部替换
- 状态栏：字符数、行数、光标行列号
- 窗口标题跟着文件的修改状态变，带星号表示还没保存
- 定时自动保存，间隔 60 秒，只对已经保存过的文件生效

## 目录

| 文件 | 作用 |
| --- | --- |
| `notepad.pro` | qmake 工程文件 |
| `mainwindow.ui` | 主窗口界面，菜单、工具栏、Action 都在 Designer 里建 |
| `mainwindow.h/.cpp` | 文件读写、编辑动作、格式设置、状态栏和查找替换的处理 |
| `codeeditor.h/.cpp` | 从 `QPlainTextEdit` 派生，左侧画行号 |
| `finddialog.*`、`replacedialog.*`、`aboutdialog.*` | 三个对话框 |
| `windowchrome.*` | 统一标题栏配色，避免跟着系统深色主题走 |
| `icons/` | 工具栏和窗口用的 SVG 图标 |
| `docs/screenshots/` | 实验报告里的运行截图 |

## 构建

本机用的是 Qt 6.11.2 和 MinGW 13.1：

```bash
qmake notepad.pro
mingw32-make -j8
```

## 快捷键

| 快捷键 | 功能 |
| --- | --- |
| Ctrl+N / Ctrl+O | 新建 / 打开 |
| Ctrl+S / Ctrl+Shift+S | 保存 / 另存为 |
| Ctrl+Z / Ctrl+Y | 撤销 / 恢复 |
| Ctrl+X / Ctrl+C / Ctrl+V | 剪切 / 复制 / 粘贴 |
| Ctrl+A | 全选 |
| Ctrl+F / Ctrl+H | 查找 / 替换 |
| Ctrl+L | 显示或隐藏行号 |
