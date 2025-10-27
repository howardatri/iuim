
# iuim
## 本项目为吉林大学面向对象课程设计，采用技术栈为golang(fyne) + node.js + c++17 + sqlite + nlonmannjson,完全实现了前后端分离
需要部署的话可以使用ngork替换客户端的ip地址，然后重新编译
### 劳驾点点star，这对我很重要，谢谢

# iuim — 混合通信架构示例 / Hybrid TCP–HTTP Microservices Demo

本项目展示了一个将 TCP 与 HTTP 结合的微服务通信架构：客户端通过 TCP 与网关交互，网关再以 HTTP 调用后端微服务。服务器端使用 C++ 的 `httplib` 作为 HTTP 服务框架，数据库采用 SQLite；客户端使用 Go；网关使用 Node.js。

This project demonstrates a hybrid communication architecture where the client talks to a gateway over TCP, and the gateway forwards requests to backend microservices over HTTP. The server side uses C++ `httplib` for HTTP services, SQLite for storage; the client is written in Go; the gateway is in Node.js.


## 架构总览 / Architecture Overview

- `Client (Go)` —TCP→ `Gateway (Node.js)` —HTTP→ `Microservices (C++/httplib)` —SQLite→ `iuim.db`
- 网关充当协议转换层：接收 TCP-Line 文本消息，转发为 HTTP 请求；微服务返回 JSON，网关再转换为 TCP 响应。
- The gateway acts as a protocol translator: it receives TCP-Line text messages, forwards them as HTTP requests to microservices; responses are JSON, which the gateway returns over TCP.


## 功能特性 / Features

- 用户：注册、登录、资料更新、绑定微信、获取资料
- 好友：添加、删除、查询、搜索、共同好友、跨服务好友推荐
- 群组：加入/退出、成员查询、创建、列表、设置与角色、子群管理等
- 服务管理：激活/停用服务、查询用户服务
- 消息：发送消息、查询历史

- Users: register, login, profile update, bind WeChat, get profile
- Friends: add, delete, query, search, common friends, cross-service recommendations
- Groups: join/quit, members, create, list, settings & roles, subgroups
- Service management: activate/deactivate service, query user services
- Messages: send, history


## 技术栈 / Tech Stack

- 后端微服务：C++17 + `httplib` + `nlohmann/json`
- 数据库：SQLite (`sqlite3.dll` 已随仓库提供)
- 网关：Node.js + `net` + `axios`
- 客户端：Go

- Backend microservices: C++17 + `httplib` + `nlohmann/json`
- Database: SQLite (bundled `sqlite3.dll`)
- Gateway: Node.js + `net` + `axios`
- Client: Go


## 目录结构 / Repository Layout

- `server/` C++ 微服务代码与构建脚本（`build_*.bat`）
- `gateway/` Node.js TCP 网关与命令路由
- `client/` Go 客户端与 UI
- `third_party/` 依赖：`httplib`、`json`、`sqlite`
- `server/iuim.db` 或 `server/data/iuim.db` SQLite 数据库文件

- `server/` C++ microservices and Windows build scripts (`build_*.bat`)
- `gateway/` Node.js TCP gateway and command routing
- `client/` Go client and UI
- `third_party/` vendored deps: `httplib`, `json`, `sqlite`
- `server/iuim.db` or `server/data/iuim.db` SQLite DB file


## 端口与进程 / Ports & Processes

- `UserSVC` (C++): `0.0.0.0:50051`
- `MsgSVC` (C++): `0.0.0.0:50052`
- `FriendSVC` (C++): `0.0.0.0:50054`
- `GroupSVC` (C++): `0.0.0.0:50055`
- `ServiceSVC` (C++): `0.0.0.0:50056`
- `Gateway` (Node.js TCP): `0.0.0.0:8233`


## 快速开始 / Quick Start

### 前置环境 / Prerequisites

- Windows（推荐），安装：`Node.js >= 18`、`Go >= 1.20`、C++ 编译器（MSVC 或 MinGW）
- SQLite 动态库已在 `third_party/sqlite/sqlite3.dll`

- Windows recommended; install `Node.js >= 18`, `Go >= 1.20`, and a C++ compiler (MSVC or MinGW)
- SQLite runtime is bundled under `third_party/sqlite/sqlite3.dll`

### 构建与运行微服务 / Build & Run Microservices (C++)

在仓库根目录的终端（PowerShell）：

- 构建（按需运行）：
  - `.\nserver\build_usersvc.bat`
  - `.
server\build_friendsvc.bat`
  - `.
server\build_groupsvc.bat`
  - `.
server\build_messagesvc.bat`
  - `.
server\build_servicesvc.bat`
- 运行可执行文件（输出位置以脚本为准），确保每个服务在上面的端口监听。

In PowerShell at repo root:

- Build (run as needed):
  - `.
server\build_usersvc.bat`
  - `.
server\build_friendsvc.bat`
  - `.
server\build_groupsvc.bat`
  - `.
server\build_messagesvc.bat`
  - `.
server\build_servicesvc.bat`
- Start each service binary and confirm it listens on its port.

### 启动网关 / Start Gateway (Node.js)

```powershell
cd gateway
npm install
node src/index.js
```

### 启动客户端 / Run Client (Go)

```powershell
cd client
go run cmd/main.go
```

### 用TCP命令快速自测 / Quick TCP Smoke Test

你也可以使用示例脚本连接到网关并发送命令：

You can use the sample script to connect to the gateway and send commands:

```powershell
cd gateway
node test_gateway.js
```

示例命令（TCP-Line 格式）：
- `health_check\n`
- `register {"username":"alice","password":"secret","email":"a@b.com"}\n`
- `get_group_settings {"group_id":123,"user_id":1}\n`

Example TCP-Line commands:
- `health_check\n`
- `register {"username":"alice","password":"secret","email":"a@b.com"}\n`
- `get_group_settings {"group_id":123,"user_id":1}\n`


## HTTP 接口概览 / HTTP Endpoints Overview

微服务通过 `httplib::Server` 注册路由并返回 JSON。例如：

The microservices register routes via `httplib::Server` and return JSON.

- `UserSVC (50051)`:
  - `POST /health-check`, `POST /register`, `POST /login`
  - `POST /update_profile`, `POST /bind_wechat`, `POST /get_profile`
- `FriendSVC (50054)`:
  - `POST /add`, `POST /delete`, `POST /query`, `POST /search`
  - `POST /common`, `POST /cross-service`
- `GroupSVC (50055)`:
  - `POST /join`, `POST /quit`, `POST /members`, `POST /create`, `POST /list`
  - 设置与角色等：`POST /group_settings`, `POST /update_group_settings`, `POST /change_group_type`, `POST /set_member_role`, `POST /get_member_roles`, `POST /apply_join_group`, `POST /invite_join_group`, `POST /free_join_topic`, `POST /get_current_user_role`
  - 子群：`POST /create_subgroup`, `POST /get_subgroups`, `POST /join_subgroup`, `POST /quit_subgroup`, `POST /delete_subgroup`, `POST /get_subgroup_members`
- `MsgSVC (50052)`:
  - `POST /send`, `POST /history`, `GET /health`
- `ServiceSVC (50056)`:
  - `POST /activate`, `POST /deactivate`, `POST /query_user_services`, `GET /health`


## 通信协议 / Communication Protocols

- TCP-Line（客户端↔网关）：
  - 请求：`<command> <JSON>` 以换行 `\n` 结束，例如：`register {"username":"alice"}\n`
  - 响应：`<command>_resp <JSON>` 或直接 JSON（例如 `health_check`），以换行结束。
- HTTP（网关↔微服务）：
  - 网关使用 `axios.post("http://localhost:<port>/<endpoint>", data)` 发送 JSON；微服务以 `application/json` 响应。

- TCP-Line (Client↔Gateway):
  - Request: `<command> <JSON>` terminated by `\n`, e.g., `register {"username":"alice"}\n`
  - Response: `<command>_resp <JSON>` or raw JSON (e.g. `health_check`), terminated by `\n`.
- HTTP (Gateway↔Microservices):
  - Gateway sends JSON with `axios.post("http://localhost:<port>/<endpoint>", data)`; services respond with `application/json`.


## SQLite 数据库 / SQLite Database

数据库文件位于：`server/iuim.db` 或 `server/data/iuim.db`。




## 代码示例 / Code Examples

- C++ 响应 JSON：
  - `res.set_content(response.dump(), "application/json");`
- Node.js 网关转发：
  - `axios.post("http://localhost:50051/register", data)`
- Go 客户端发送命令：
  - `conn.Write([]byte(cmd + "\n"))`，并按行读取响应。

- C++ JSON response:
  - `res.set_content(response.dump(), "application/json");`
- Node.js gateway forwarding:
  - `axios.post("http://localhost:50051/register", data)`
- Go client sending command:
  - `conn.Write([]byte(cmd + "\n"))`, read response by line.


## 开发提示 / Development Notes

- 若需要修改端口，更新对应 `gateway/src/command-handlers/*-commands.js` 与服务 `*_main.cpp`。
- 如果遇到 JSON 解析或跨平台编码问题，统一使用 UTF-8 并确保 `Content-Type: application/json`。
- Windows 下建议用 PowerShell 执行脚本；如使用 MinGW，注意链接 `sqlite3`。

- If you change ports, update `gateway/src/command-handlers/*-commands.js` and service `*_main.cpp` accordingly.
- Use UTF-8 consistently and ensure `Content-Type: application/json` to avoid parsing issues.
- On Windows, prefer PowerShell for scripts; with MinGW ensure proper linking to `sqlite3`.


## 许可证 / License

此仓库用于课程设计与演示，未附带开源许可证。如需对外发布，请自行添加合适的许可证文件。

This repository is for coursework and demonstration; no OSS license is included. Please add an appropriate license if publishing.
