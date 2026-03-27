# 项目介绍

这个项目是便于快速部署和统一环境使用，目的是在不适用docker的情况下快速的在本地linux系统中布置可复现，可重用的环境

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