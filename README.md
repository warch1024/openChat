# LVGL-openChat

基于 **LVGL (Light and Versatile Graphics Library)** 的嵌入式TCP聊天应用项目，支持文本消息、文件传输和表情发送功能。

## 项目简介

LVGL-openChat 是一个运行在 Linux 嵌入式平台上的轻量级聊天应用，采用 C 语言开发，集成了 LVGL 图形库实现现代化的图形界面。项目包含完整的客户端-服务器架构，支持多客户端在线聊天、文件传输等功能。

## 项目结构

```
LVGL-openChat/
├── lvgl/                    # LVGL 图形库核心代码（子模块）
│   ├── demos/               # LVGL 演示程序
│   ├── docs/                # LVGL 文档
│   ├── examples/            # LVGL 使用示例
│   └── src/                 # LVGL 核心源码
├── lv_drivers/              # LVGL 设备驱动（子模块）
│   ├── display/             # 显示驱动（fbdev、DRM、SDL等）
│   └── indev/               # 输入设备驱动（evdev、触摸屏等）
├── openChat/                # 聊天应用原始版本
│   ├── resources/           # 资源文件（图片、背景等）
│   ├── chat_func.c/h        # 聊天功能实现
│   ├── login_screen.c/h     # 登录界面
│   ├── main_screen.c/h      # 主聊天界面
│   ├── openChat.c/h         # 应用入口
│   └── open_chat_client.c/h # 网络客户端
├── openChat_rebuild/        # 聊天应用重建版本（主开发分支）
│   ├── core/                # 核心模块
│   │   ├── client.c/h       # TCP客户端通信
│   │   └── user_manager.c/h # 用户管理
│   ├── resources/           # 资源文件
│   │   ├── fonts/           # 中文字体
│   │   └── *.bmp            # 图片资源
│   ├── screen/              # 界面模块
│   │   ├── login_screen.c/h # 登录界面
│   │   └── main_screen.c/h  # 主聊天界面
│   ├── tools/               # 工具函数
│   │   └── tools.c/h        # 通用工具
│   └── openchat.c/h         # 应用主入口
├── server/                  # TCP聊天服务器
│   ├── server.c             # 服务器主程序
│   ├── client1.c            # 命令行客户端
│   ├── client2.c            # GUI客户端
│   ├── myhead.h             # 公共头文件
│   └── tools.c              # 服务器工具函数
├── other-tmp-file/          # 临时文件和测试代码
├── lv_conf.h                # LVGL 配置文件
├── lv_drv_conf.h            # LVGL 驱动配置文件
├── Makefile                 # 项目构建脚本
└── LICENSE                  # 许可证文件
```

## 功能特性

### 客户端功能

1. **登录系统**
   - 用户登录验证
   - 用户信息持久化存储

2. **聊天功能**
   - 多客户端在线列表显示
   - 一对一文本消息发送
   - 消息气泡界面展示
   - 消息滚动查看

3. **文件传输**
   - 支持任意文件发送
   - 文件接收与保存
   - 图片/表情包显示

4. **表情发送**
   - 表情包预览选择
   - 一键发送表情图片

5. **中文输入**
   - 集成拼音输入法
   - 虚拟键盘交互

### 服务器功能

1. **多客户端管理**
   - 支持多个客户端同时连接
   - 在线客户端列表维护
   - 客户端连接状态监控

2. **消息转发**
   - C2C（客户端到客户端）消息转发
   - 消息格式解析与路由

3. **文件中转**
   - 文件数据接收与转发
   - 大文件分片传输支持

## 技术架构

### 核心组件

| 组件 | 职责 | 技术实现 |
|------|------|----------|
| **LVGL** | 图形界面渲染 | 嵌入式图形库 |
| **TCP Socket** | 网络通信 | Linux socket API |
| **Pthread** | 多线程处理 | POSIX线程库 |
| **Freetype** | 字体渲染 | 开源字体引擎 |

### 通信协议

消息格式采用命令头+数据体结构：

```
命令头#数据体
```

**支持的命令类型：**
- `c2c$chat#` - 客户端间聊天消息
- `send$file#` - 文件传输
- `get$all$online$clients#` - 获取在线客户端列表
- `clients$info#` - 客户端信息同步

## 构建方法

### 环境要求

- **交叉编译工具链**: `arm-linux-gcc`
- **依赖库**: `libfreetype`, `libpthread`, `libm`
- **LVGL版本**: 8.x 或更高

### 编译步骤

```bash
# 克隆项目（包含子模块）
git clone https://github.com/warch1024/openChat.git
cd openChat
git submodule update --init --recursive

# 编译项目
make

# 安装到系统
sudo make install
```

### 运行应用

```bash
# 运行演示程序
demo

# 运行服务器（在server目录）
cd server
gcc server.c tools.c -o server -lpthread
./server
```

### 项目配置

**LVGL 配置**: `lv_conf.h`
- 屏幕分辨率设置
- 颜色深度配置
- 内存分配策略

**驱动配置**: `lv_drv_conf.h`
- 显示驱动选择（fbdev/DRM/SDL）
- 输入设备配置

**服务器配置**: `server/server.c`
- 监听端口（默认：30000）
- 客户端连接限制

## 开发说明

### 代码规范

- 采用 ANSI C (C99) 标准
- 使用 `oc_` 前缀标识项目特有函数
- 函数命名采用下划线分隔（snake_case）
- 头文件包含保护使用 `#ifndef ... #define ... #endif`

### 调试技巧

```bash
# 启用调试模式
make CFLAGS="-O0 -g"

# 使用 gdb 调试
arm-linux-gdb demo
```

### 目录职责

| 目录 | 说明 |
|------|------|
| `lvgl/` | LVGL 图形库，作为子模块引入 |
| `lv_drivers/` | LVGL 设备驱动，支持多种显示/输入设备 |
| `openChat_rebuild/` | 主开发分支，包含完整的GUI聊天客户端 |
| `server/` | TCP服务器实现，支持消息转发和文件传输 |

## 许可证

项目包含以下开源组件：

- **LVGL**: MIT License
- **LVGL Drivers**: MIT License
- **OpenChat**: MIT License

详见各目录下的 LICENSE 文件。

## 贡献指南

欢迎提交 Issue 和 Pull Request！

1. Fork 本仓库
2. 创建功能分支
3. 提交代码
4. 发起 Pull Request

## 联系方式

如有问题或建议，请通过以下方式联系：

- GitHub Issues: https://github.com/warch1024/openChat/issues
