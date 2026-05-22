# LVGL-openChat

基于 LVGL (Light and Versatile Graphics Library) 的嵌入式聊天应用项目。

## 项目结构

```
LVGL-openChat/
├── lvgl/           # LVGL 图形库核心代码
├── lv_drivers/     # LVGL 显示和输入设备驱动
├── openChat/       # 聊天应用原始版本
├── openChat_rebuild/ # 聊天应用重建版本（主开发分支）
└── other-tmp-file/  # 临时文件和测试代码
```

## 主要组件

### lvgl/
- 嵌入式图形库核心
- 支持多种显示驱动和输入设备
- 提供丰富的UI组件（按钮、列表、标签等）

### lv_drivers/
- 显示驱动（fbdev、DRM、SDL等）
- 输入设备驱动（evdev、键盘、触摸屏等）

### openChat_rebuild/
- 聊天应用主程序
- 登录界面
- 主聊天界面
- 用户管理
- 网络客户端

## 构建项目

```bash
make
sudo make install
```

## 运行应用

```bash
demo
```

## 开发说明

项目使用 ARM Linux 交叉编译工具链构建。主要源文件位于 `openChat_rebuild/` 目录。

## 许可证

项目包含 LVGL 库及其驱动，遵循各自的开源许可证。