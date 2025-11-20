# Chess Studio

Chess Studio 是一个以国际象棋为核心、集对弈、研讨、复盘与学习于一体的智能 Agent。
项目在 Qt/C++ 图形栈上演进而来，兼容图形界面、命令行以及可复用的库组件，帮助俱乐部、
教学工作坊以及线上社区搭建面向国际象棋的数字工作台。

## Why Chess Studio?

- 统一的 Agent 身份：一个入口完成国际象棋对弈、讲解、检索与分享
- 友好的桌面级 GUI，适合新手入门也支持进阶战术分析
- 扩展性强的 C++/Qt 架构，方便继续集成更多引擎或学习工具

## Feature Highlights

- AI 难度调节：内置 5 档 Stockfish Skill Level 预设，适配从初学到高手
- 一键悔棋：随时回退到任意一步，用于讲解或重新规划
- 复盘模式：自动生成复盘时间轴，支持复现关键局面和批注
- 社群互动：提供研究房间/讨论布局，方便教练与同学实时交流
- 偏好持久化：自动记忆常用引擎、主题和学习布局

## Getting Started

### 必备环境

所有依赖详见 `REQUIREMENTS.md`。核心依赖包括：

- Qt ≥ 5.15（Widgets、SVG、Concurrent、PrintSupport、Testlib）
- 支持 C++17 的编译器（Clang、GCC 或 MSVC）
- CMake ≥ 3.18 以及 Ninja 或 Make

### 构建

```bash
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --target gui
```

### 启动 Agent

- 图形界面：运行 `./build/cutechess` 即可进入 Chess Studio 体验
- 命令行：`./build/cutechess-cli` 适用于批量对局、引擎测试与脚本化复盘

在"New Game"对话框切换任一方为 CPU 时会自动展示 AI Difficulty 面板，可直接在
五个预设难度之间切换。悔棋、复盘入口位于主工具栏与对局菜单中。

## Packaging Chess Studio 1.0

1. 使用 Release 配置重新构建：`cmake --build build --target gui --config Release`
2. 将可执行文件、`dist/` 资源与必需的 Qt 运行库收集到 `dist/ChessStudio-1.0/`
3. 运行 `cmake --build build --target package` 或手动压缩为 `ChessStudio-1.0.zip`
4. 在 GitHub 创建 `v1.0.0` 标签并上传压缩包与校验信息

## Documentation & Support

- 设计文档与 API 说明位于 `docs/` 目录
- Stockfish 难度调节、悔棋与复盘功能的技术细节可在设计文档中查阅
- 将仓库同步至 GitHub 后，可通过 Issues / Discussions 收集反馈
- 系统自动加载 `stockfish/` 子项目提供的最新引擎构建，可直接作为默认对弈 AI

## License

- 主体代码遵循 GPLv3+
- `projects/lib/components` 与 `projects/gui/components` 目录下的组件为 MIT 协议

## Credits

Chess Studio 基于 Cute Chess 社区（Ilari Pihlajisto、Arto Jonsson 及所有贡献者）的
多年积累进行再封装，特别感谢原项目提供的能力与灵感。
同样感谢 Stockfish 项目全体维护者与贡献者提供卓越的开源棋力引擎，使 Chess Studio
能够即装即用地为玩家带来强大的对弈伙伴。
