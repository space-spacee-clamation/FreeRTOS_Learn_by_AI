# FreeRTOSDemo - STM32平台FreeRTOS渐进式学习项目

## 项目整体介绍

这是一个**STM32平台FreeRTOS渐进式学习项目集合**，旨在通过动手实践的方式，逐步深入理解FreeRTOS实时操作系统的核心概念和设计思想。项目特点：

- 📚 **渐进式学习**：从环境搭建开始，由浅入深，逐步深入核心功能
- 💻 **实践驱动**：每个知识点都配套可运行的示例项目
- 📝 **经验沉淀**：记录学习过程中遇到的问题和解决方案，便于后续查阅复用
- 🤖 **AI友好**：清晰的文档结构和索引，便于AI辅助开发和维护

本项目支持在WSL/Linux环境下开发，可通过QEMU进行仿真测试，也可编译下载到真实STM32硬件运行。

## 目录结构总览

```
FreeRTOSDemo/
├── Readme.md          # 项目介绍与快速开始（本文件）
├── 项目索引.md         # 完整项目索引，快速导航
├── init.sh            # 环境初始化脚本
├── packages.txt       # 依赖包列表
├── .gitignore         # Git忽略配置
│
├── plan/              # 学习规划目录
│   ├── 学习流程规划.md   # 完整的分阶段学习规划
│   └── 当前进度.md       # 当前学习进度跟踪
│
├── projects/          # 学习项目集合
│   └── stm32-learning/  # STM32平台渐进式学习项目
│       └── 按章节编号组织各模块实践项目
│
└── doc/              # 文档与经验沉淀
    ├── Git规范.md     # Git分支命名和提交规范
    ├── 目录说明.md     # doc目录详细说明
    ├── experiences/   # 按技术分类的经验沉淀
    │   ├── stm32/     # STM32开发经验
    │   ├── freertos/  # FreeRTOS相关经验
    │   ├── build-system/  # 编译构建经验
    │   └── qemu/      # QEMU仿真经验
    └── tutorial/      # 教程文档
        └── Mastering-the-FreeRTOS-Kernel/  # 《Mastering the FreeRTOS》书籍笔记
```

### 各顶级目录用途说明

| 目录 | 用途 |
|------|------|
| `plan/` | 存放学习规划和进度跟踪文档，定义整体学习路线和当前进展 |
| `projects/` | 存放所有可编译运行的实践项目代码，按平台和学习阶段组织 |
| `doc/` | 存放所有文档：包括开发规范、经验沉淀、教程笔记等 |

项目遵循**文档与代码分离**原则：文档统一在`doc/`维护，代码在`projects/`组织，便于独立更新和检索。

## 渐进式学习思路

本项目采用**分阶段渐进式**学习方法：

### 第一阶段：环境搭建与基础验证
- 目标：完成开发环境搭建，验证编译工具链能够正常工作
- 输出：基础STM32项目模板，可成功编译链接FreeRTOS内核

### 第二阶段：基础知识与入门项目
- 目标：掌握FreeRTOS基础知识，创建简单入门示例
- 内容：任务创建、任务调度、队列通信、信号量使用等基础功能实践

### 第三阶段：深入核心功能与重点研究
- 目标：深入理解FreeRTOS核心模块，重点研究三大模块：
  1. **堆内存管理** - 五种内存分配方案的分析与实践
  2. **软件定时器** - 定时器工作原理与应用实践
  3. **流缓冲区** - 中断与任务间高效通信实践

### 后续阶段：高级功能与实际项目
- 根据学习进度，可以继续扩展高级功能（如事件组、任务通知等）
- 最终可以构建一个完整的实际应用项目

## 如何开始学习

### 1. 环境准备

如果你使用Windows，建议先安装WSL：

```bash
# 在PowerShell中执行
wsl --install --location D:\WSL\Ubuntu
```

重启后进入WSL终端，继续下一步。

如果你已经使用Linux，可以直接跳过这一步。

### 2. 获取项目

```bash
git clone https://git.code.tencent.com/Space0021/FreeRTOSDemo.git
cd FreeRTOSDemo
code .  # 在VSCode中打开项目
```

### 3. 自动初始化环境

在WSL/Linux终端中运行：

```bash
sh ./init.sh
```

该脚本会自动安装所需的编译工具和依赖，并获取FreeRTOS官方源码。

### 4. 开始学习

1. **阅读规划**：先阅读 [`plan/学习流程规划.md`](plan/学习流程规划.md) 了解整体学习路线
2. **查看进度**：查看 [`plan/当前进度.md`](plan/当前进度.md) 了解当前学到哪里
3. **选择项目**：进入 [`projects/stm32-learning/`](projects/stm32-learning/) 选择对应阶段的项目进行学习
4. **查阅经验**：遇到问题可以到 [`doc/experiences/`](doc/experiences/) 查找相关经验
5. **沉淀经验**：解决问题后，将新的经验总结存入 `doc/experiences/` 对应分类中

### 5. 快速导航

完整的导航索引请参见 [`项目索引.md`](项目索引.md) 文件，包含所有重要文档的快速链接。

## 初始化wsl

如果您还没有安装WSL，可以按照以下步骤创建WSL虚拟机：

1. 打开PowerShell或Windows命令提示符，执行以下命令进行安装：
```bash
wsl --install --location D:\WSL\Ubuntu
```

> **注意**：请使用`--location`参数指定WSL的安装路径，避免使用默认的系统盘路径，以节省系统盘空间。您可以根据自己的实际情况修改安装路径。

2. 安装完成后，按照提示重启计算机。
3. 重启后，WSL会自动启动，按照提示设置用户名和密码即可完成初始化。

如果您已经安装了WSL，可以直接打开WSL终端继续下一步操作。

## 拉取项目

打开WSL终端后，使用以下命令拉取项目：

```bash
git clone https://git.code.tencent.com/Space0021/FreeRTOSDemo.git
```

进入项目目录：

```bash
cd FreeRTOSDemo
```

您可以使用以下命令在VSCode中打开项目：

```bash
code .
```

## 环境初始化

在WSL终端中，进入项目目录后，运行初始化脚本：

```bash
sh ./init.sh
```

该脚本会自动完成以下操作：
1. 使用`sudo apt install`安装编译工具和依赖包，包括`gcc`、`make`等
2. 执行`git clone https://github.com/FreeRTOS/FreeRTOS.git --recurse-submodules`获取官方FreeRTOS源码

> **注意**：FreeRTOS文件夹作为外部依赖，不会被提交到本项目仓库，已经添加到`.gitignore`中。如果您手动克隆FreeRTOS源码，请确保将其添加到`.gitignore`文件中。