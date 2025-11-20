# Stockfish 难度调整功能实现文档

**日期**: 2025年11月20日  
**项目**: Chess Studio Agent GUI  
**功能**: Stockfish AI 引擎难度选择功能

---

## 目录

1. [功能概述](#功能概述)
2. [需求分析](#需求分析)
3. [技术调研](#技术调研)
4. [实现方案](#实现方案)
5. [代码修改详情](#代码修改详情)
6. [使用方法](#使用方法)
7. [技术细节](#技术细节)

---

## 功能概述

为 Chess Studio Agent 添加了 Stockfish AI 引擎的难度调整功能，允许用户根据自身水平选择合适的 AI 难度级别，使新手玩家能够更好地学习和练习国际象棋。

### 核心功能

- ✅ 在新游戏对话框中添加难度选择控件
- ✅ 提供 5 个预设难度级别（新手、业余、中级、高级、最强）
- ✅ 自动检测 Stockfish 引擎并应用难度设置
- ✅ 保存用户偏好设置
- ✅ 智能显示/隐藏难度选择（仅在选择 CPU 玩家时显示）

---

## 需求分析

### 用户需求

用户反馈：**"Stockfish 的 AI 太强大了，不适合新手学习国际象棋，有没有办法调整一下 Stockfish 的难度？"**

### 需求要点

1. 允许用户根据能力选择 AI 的难易程度
2. 为新手、业余、职业棋手提供更好的匹配
3. 简化难度设置流程，无需手动配置引擎选项

---

## 技术调研

### Stockfish 难度选项

通过代码库搜索和文档查阅，发现 Stockfish 支持以下难度相关选项：

#### 1. Skill Level（技能等级）
- **类型**: `spin` (整数)
- **范围**: 0 - 20
- **默认值**: 20（最强）
- **说明**: 降低 Skill Level 可以让 Stockfish 变弱。内部会启用 MultiPV，并根据 Skill Level 以一定概率选择较弱的走法。

#### 2. UCI_LimitStrength（限制强度）
- **类型**: `check` (布尔值)
- **默认值**: `false`
- **说明**: 启用后可以通过 UCI_Elo 设置目标 Elo 等级。此选项会覆盖 Skill Level。

#### 3. UCI_Elo（Elo 等级）
- **类型**: `spin` (整数)
- **范围**: 1320 - 3190
- **默认值**: 1320
- **说明**: 当 UCI_LimitStrength 启用时，设置目标 Elo 等级。

### 技术选择

**选择使用 Skill Level 而非 UCI_Elo 的原因：**
- Skill Level 更直观，范围 0-20 易于理解
- 不需要启用 UCI_LimitStrength，配置更简单
- 对于新手用户更友好

---

## 实现方案

### UI 设计

在"新游戏"对话框中添加"AI Difficulty"组框，包含：
- 标签："Difficulty Level:"
- 下拉选择框：包含 5 个预设难度级别
- 工具提示：说明功能用途

### 难度级别映射

| 显示名称 | Skill Level | 适用对象 |
|---------|------------|---------|
| Beginner (Easy) | 5 | 新手玩家 |
| Amateur (Medium) | 12 | 有一定基础的玩家 |
| Intermediate | 16 | 中级玩家 |
| Advanced | 18 | 高级玩家 |
| Maximum (Strongest) | 20 | 职业玩家（默认） |

### 实现逻辑

1. **检测引擎类型**：在 `preparedEngineConfiguration()` 中检测是否为 Stockfish 引擎
2. **应用难度设置**：如果是 Stockfish，设置 `Skill Level` 选项
3. **禁用冲突选项**：禁用 `UCI_LimitStrength` 确保 Skill Level 生效
4. **保存用户偏好**：将选择的难度保存到 QSettings

---

## 代码修改详情

### 1. UI 文件修改 (`projects/gui/ui/newgamedlg.ui`)

**添加难度选择组框：**

```xml
<widget class="QGroupBox" name="m_difficultyGroupBox">
 <property name="title">
  <string>AI Difficulty</string>
 </property>
 <property name="toolTip">
  <string>Adjust the AI engine strength for better match with your skill level</string>
 </property>
 <layout class="QHBoxLayout" name="horizontalLayout_4">
  <item>
   <widget class="QLabel" name="m_difficultyLabel">
    <property name="text">
     <string>Difficulty Level:</string>
    </property>
   </widget>
  </item>
  <item>
   <widget class="QComboBox" name="m_difficultyComboBox">
    <property name="toolTip">
     <string>Select AI difficulty level. Lower levels are easier for beginners.</string>
    </property>
   </widget>
  </item>
  ...
 </layout>
</widget>
```

**调整对话框高度：** 从 242 增加到 280 像素

### 2. 头文件修改 (`projects/gui/src/newgamedlg.h`)

**添加难度级别获取方法：**

```cpp
int difficultyLevel() const;
```

**添加辅助方法：**

```cpp
void updateDifficultyVisibility();
```

### 3. 实现文件修改 (`projects/gui/src/newgamedlg.cpp`)

#### 3.1 初始化难度选择控件

```cpp
// Setup difficulty levels
ui->m_difficultyComboBox->addItem(tr("Beginner (Easy)"), 5);
ui->m_difficultyComboBox->addItem(tr("Amateur (Medium)"), 12);
ui->m_difficultyComboBox->addItem(tr("Intermediate"), 16);
ui->m_difficultyComboBox->addItem(tr("Advanced"), 18);
ui->m_difficultyComboBox->addItem(tr("Maximum (Strongest)"), 20);

// Set default to Maximum
ui->m_difficultyComboBox->setCurrentIndex(4);

// Load saved difficulty preference
QSettings settings;
int savedDifficulty = settings.value("games/difficulty_level", 20).toInt();
for (int i = 0; i < ui->m_difficultyComboBox->count(); i++)
{
    if (ui->m_difficultyComboBox->itemData(i).toInt() == savedDifficulty)
    {
        ui->m_difficultyComboBox->setCurrentIndex(i);
        break;
    }
}

// Save difficulty when changed
connect(ui->m_difficultyComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
    [=](int index)
{
    int level = ui->m_difficultyComboBox->itemData(index).toInt();
    QSettings().setValue("games/difficulty_level", level);
});

// Initially hide difficulty group
ui->m_difficultyGroupBox->setVisible(false);

// Update difficulty visibility when player type changes
connect(ui->m_whitePlayerCpuRadio, &QRadioButton::toggled, [=](bool checked)
{
    updateDifficultyVisibility();
});
connect(ui->m_blackPlayerCpuRadio, &QRadioButton::toggled, [=](bool checked)
{
    updateDifficultyVisibility();
});
```

#### 3.2 应用难度设置到引擎配置

```cpp
EngineConfiguration NewGameDialog::preparedEngineConfiguration(Chess::Side side) const
{
    auto config = m_engineConfig[side];
    ui->m_gameSettings->applyEngineConfiguration(&config);
    
    // Apply difficulty setting for Stockfish and other engines that support Skill Level
    // Check if the engine name contains "Stockfish" (case-insensitive)
    QString engineName = config.name();
    if (engineName.contains("Stockfish", Qt::CaseInsensitive))
    {
        int skillLevel = difficultyLevel();
        config.setOption("Skill Level", skillLevel);
        
        // Disable UCI_LimitStrength to use Skill Level instead
        config.setOption("UCI_LimitStrength", false);
    }
    
    return config;
}
```

#### 3.3 获取难度级别

```cpp
int NewGameDialog::difficultyLevel() const
{
    int index = ui->m_difficultyComboBox->currentIndex();
    if (index >= 0 && index < ui->m_difficultyComboBox->count())
        return ui->m_difficultyComboBox->itemData(index).toInt();
    return 20; // Default to maximum
}
```

#### 3.4 更新难度选择可见性

```cpp
void NewGameDialog::updateDifficultyVisibility()
{
    bool hasCpuPlayer = (playerType(Chess::Side::White) == CPU) ||
                        (playerType(Chess::Side::Black) == CPU);
    ui->m_difficultyGroupBox->setVisible(hasCpuPlayer);
}
```

#### 3.5 在 setPlayerType 中调用可见性更新

```cpp
void NewGameDialog::setPlayerType(Chess::Side side, PlayerType type)
{
    // ... existing code ...
    updateDifficultyVisibility();
}
```

---

## 使用方法

### 步骤说明

1. **打开新游戏对话框**
   - 点击菜单 "Game" → "New Game"
   - 或使用快捷键

2. **选择玩家类型**
   - 将 White 或 Black（或两者）设置为 "CPU"
   - 难度选择控件会自动显示

3. **选择引擎**
   - 从下拉框中选择 "Stockfish" 引擎

4. **选择难度级别**
   - 在 "AI Difficulty" 组框中选择合适的难度：
     - **Beginner (Easy)**: 适合初学者
     - **Amateur (Medium)**: 适合有一定基础的玩家
     - **Intermediate**: 适合中级玩家
     - **Advanced**: 适合高级玩家
     - **Maximum (Strongest)**: Stockfish 的完整强度（默认）

5. **开始游戏**
   - 点击 "OK" 开始游戏
   - AI 将按照选择的难度级别对局

### 注意事项

- 难度设置仅对 Stockfish 引擎生效
- 如果选择其他引擎，难度设置不会应用
- 难度偏好会自动保存，下次打开对话框时会自动加载

---

## 技术细节

### Stockfish Skill Level 工作原理

根据 Stockfish 源代码分析：

1. **MultiPV 启用**：当 Skill Level < 20 时，内部会启用 MultiPV 搜索
2. **概率选择**：根据 Skill Level 的值，以一定概率选择次优走法
3. **强度递减**：Skill Level 越低，选择较弱走法的概率越高

### 配置存储

- **存储位置**: QSettings
- **键名**: `games/difficulty_level`
- **类型**: 整数 (5, 12, 16, 18, 20)
- **默认值**: 20 (最强)

### 引擎选项设置

使用 `EngineConfiguration::setOption()` 方法设置引擎选项：

```cpp
config.setOption("Skill Level", skillLevel);
config.setOption("UCI_LimitStrength", false);
```

这些选项会在引擎启动时通过 UCI 协议发送给 Stockfish。

### UCI 协议命令

实际发送给 Stockfish 的命令格式：

```
setoption name Skill Level value 5
setoption name UCI_LimitStrength value false
```

---

## 相关文件清单

### 修改的文件

1. `projects/gui/ui/newgamedlg.ui` - UI 布局文件
2. `projects/gui/src/newgamedlg.h` - 头文件
3. `projects/gui/src/newgamedlg.cpp` - 实现文件

### 参考的文件

1. `stockfish/src/engine.cpp` - Stockfish 引擎选项定义
2. `stockfish/wiki/UCI-&-Commands.md` - UCI 协议文档
3. `projects/lib/src/engineconfiguration.cpp` - 引擎配置实现
4. `projects/gui/src/engineconfigurationdlg.cpp` - 引擎配置对话框

---

## 测试建议

### 功能测试

1. ✅ 验证难度选择控件正确显示/隐藏
2. ✅ 验证不同难度级别的设置是否正确应用
3. ✅ 验证用户偏好是否正确保存和加载
4. ✅ 验证仅对 Stockfish 引擎生效
5. ✅ 验证其他引擎不受影响

### 游戏测试

1. 使用不同难度级别与 AI 对局
2. 观察 AI 走法质量是否符合预期
3. 验证难度设置是否在游戏过程中生效

---

## 未来改进建议

1. **支持更多引擎**：扩展难度设置支持其他支持 Skill Level 的引擎
2. **自定义难度**：允许用户手动输入 Skill Level 值（0-20）
3. **Elo 模式**：添加基于 Elo 等级的难度选择选项
4. **难度预览**：显示每个难度级别对应的近似 Elo 等级
5. **自适应难度**：根据玩家表现自动调整难度

---

## 编译和构建

### 编译命令

```bash
cd /Users/biaowenhuang/Documents/cutechess-master
cmake --build build --target gui
```

### 编译状态

✅ **编译成功** - 所有修改已通过编译验证

---

## 总结

成功实现了 Stockfish AI 引擎的难度调整功能，为不同水平的玩家提供了更好的游戏体验。功能设计简洁直观，易于使用，同时保持了代码的可维护性和扩展性。

### 主要成就

- ✅ 用户友好的难度选择界面
- ✅ 自动检测和应用难度设置
- ✅ 用户偏好持久化
- ✅ 智能 UI 显示/隐藏
- ✅ 代码质量良好，编译通过

---

**文档生成时间**: 2025年11月20日  
**作者**: AI Assistant  
**项目**: Cute Chess GUI Enhancement

