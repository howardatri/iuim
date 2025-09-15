# IUIM - 集成统一即时通讯系统

## 项目概述

IUIM（Integrated Unified Instant Messaging）是一个集成统一的即时通讯系统，旨在提供一个统一的平台，集成多种即时通讯服务，使用户能够通过单一界面管理和使用多个通讯账号。

## 项目结构

```
iuim/
├── client/                 # 客户端（Golang + Fyne）
│   ├── cmd/                # 命令行入口
│   │   ├── main.go        # 主程序入口
│   │   └── test/          # 测试程序
│   └── internal/          # 内部包
│       ├── models/        # 数据模型
│       ├── network/       # 网络通信
│       ├── services/      # 服务管理
│       └── ui/            # 用户界面
└── server/                # 服务端（C++）
    ├── include/           # 头文件
    │   ├── base/          # 基础组件
    │   └── models/        # 数据模型
    ├── src/               # 源文件
    │   ├── base/          # 基础组件实现
    │   └── models/        # 数据模型实现
    └── test/              # 测试程序
```

## 功能特性

- 多服务集成：支持集成多种即时通讯服务
- 统一界面：提供统一的用户界面管理多个通讯账号
- 跨平台支持：基于Golang和Fyne的客户端支持多平台运行
- 可扩展架构：模块化设计，易于扩展新的服务类型

## 开发环境

### 客户端

- Golang 1.24+
- Fyne 2.6+
- gorilla/net.dial

### 服务端

- C++11 或更高版本
- winsock
- 支持多平台（Windows、Linux、macOS）

## 构建与运行

### 客户端

```bash
# 进入客户端目录
cd client

# 安装依赖
go mod tidy

# 构建
go build -o bin/iuim-client cmd/main.go

# 运行
./bin/iuim-client
```

### 服务端

```bash
# 进入服务端测试目录
cd server/test


## 测试

### 基础功能测试

```bash
# 进入客户端测试目录
cd client/cmd/test

# 运行测试程序
go run main.go
```

## 项目状态

当前项目处于开发阶段，已完成：

- [x] 服务端基础框架
- [x] 客户端基础框架
- [x] 服务端用户和群组基类
- [x] 客户端UI基础界面
- [x] 服务端网络通信基础
- [x] 客户端网络通信基础
- [ ] 基础功能测试
- [ ] 完整功能实现

## 许可证

[MIT License](LICENSE)