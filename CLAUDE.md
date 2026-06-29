# CLAUDE.md

## 项目是什么

这是电赛备赛仓库。当前重点顶层文件夹：

- `G-circuit-model/`：G题「电路模型探究装置」
- `control-car/`：MSPM0G3507 循迹小车训练平台
- 主控平台：TI MSPM0G3507

## 先怎么查

1. 先读根目录 `README.md`，确认仓库结构。
2. 再读 `比赛文档/00-总要求.md`，确认通用要求。
3. 再读 `比赛文档/01-MSPM0G3507训练计划.md`，确认当前训练计划。
4. 处理 G 题时，只进入 `G-circuit-model/`。
5. 处理循迹小车、电机、编码器、PID、通信、显示、控制类训练时，进入 `control-car/`。
6. 处理小车任务时优先读 `control-car/docs/AI-IMPLEMENTATION-PLAN.md`。
7. 查通用算法、通用硬件积木时，看 `common/`。
8. **写小车代码前**，必须读 `control-car/hardware/pinmap.md`（真实引脚映射）。
9. **调 PID 参数前**，必须读 `control-car/firmware/mspm0/config/pid-defaults.md`（起步参数）。

## 视觉平台

- **首选**：星瞳 OpenMV H7 Plus（STM32H743，MicroPython，内置色块/AprilTag/巡线/二维码算法）
- **备选**：K230 + CSI 摄像头（深度学习分类场景，需提前训模型转 kmodel）
- **调试**：USB 摄像头 + PC OpenCV（算法预研和参数调优，不用于比赛现场）
- OpenMV 通过 UART 发送目标坐标给 MSPM0，MSPM0 控制云台对准

## 云台平台

- **开环步进**：DCC-100v3 驱动模块 + 42 步进电机（PA12~PA16，定时器脉冲控制）
- **闭环步进**：DCC-101v1 驱动模块（UART 协议控制，AA 55 帧格式）
- **二维云台**：两路步进电机 + 按键上下左右遥控（参考 `【云台】08`）
- **激光笔**：PA17 控制，长按按键开关
- 步进电机分辨率：1 脉冲 = 0.05625°（360°/6400 微步）
- 云台引脚已整理到 `control-car/hardware/gimbal-pinmap.md`

## 参考已实现案例

### 小车控制（地猛星 MSPM0G3507 CCS 工程）
```
D:\work\参考文件-已实现案例\2026_04_地猛星电赛控制题配套资料\
```

| 参考工程 | 已实现内容 | 关键文件 |
|---------|-----------|---------|
| `【小车】01_DC_MOTOR_PID_1_弄好了基本的旋转控制/` | TB6612 单路 PWM + 方向控制 | `user_driver/motor.c/h` |
| `【小车】02_DC_MOTOR_PID_2_可以测速了/` | 编码器 GPIO 中断计数 + 速度计算 | `user_driver/motor.c` |
| `【小车】02_DC_MOTOR_PID_3_已完成一路电机PID闭环控制/` | 增量式 PI 速度闭环 | `user_driver/motor.c`（含 PID） |
| `【小车】03_PID_car_灰度模块小车巡线/` | **双电机 + 5路灰度循迹**（最完整） | `user_driver/motor.c/h`, `user_driver/huidu.c/h` |
| `【小车】04_MPU6050_DMP读取角度/` | MPU6050 DMP 角度 | `user_driver/` |
| `【小车】06_读取串口陀螺仪数据并显示在OLED屏幕上/` | 串口陀螺仪 + OLED | `user_driver/` |

### 云台控制（地猛星 MSPM0G3507 CCS 工程）

| 参考工程 | 已实现内容 | 关键文件 |
|---------|-----------|---------|
| `【云台】02_step_motor_已完成_可以转起来/` | DCC-100v3 开环步进电机 GPIO 控制 | `step_motor.c/h` |
| `【云台】03_step_motor_已完成_可以控制速度和角度/` | **定时器脉冲频率控制角速度 + 角度定位** | `step_motor.c/h` |
| `【云台】04_闭环步进电机角度控制/` | DCC-101v1 闭环 UART 协议控制 | `main.c`（AA 55 帧） |
| `【云台】05_地猛星驱动SPI屏幕_已完成/` | SPI OLED 中文显示 | `empty.c`, `oled.c/h` |
| `【云台】07_按键长按控制激光笔/` | 按键消抖 + 长按检测 + 激光开关（PA17） | `empty.c`, `key.c/h` |
| `【云台】08_按键上下左右遥控开环电机/` | **双轴云台按键遥控 + 激光笔** （最完整） | `empty.c`, `step_motor.c/h`, `key.c/h` |

### 视觉 / AI
```
D:\work\参考文件-已实现案例\2025_09_yolo全系列使用教程_2025_09\     # YOLO 训练 pipeline（train.py, data.yaml, data_split.py）
D:\work\参考文件-已实现案例\2025_10_k230运行yolov12\               # K230 端侧 YOLOv12 推理（yolov12_已完成.py, best.kmodel）
D:\work\参考文件-已实现案例\2026_06_电赛视觉资料\                  # 双路 MJPEG 网页推流（xbhdcc_tools.py）
```

### DCC 驱动文档
```
D:\work\参考文件-已实现案例\DCC-100双路步进电机驱动模块资料(2)\
  ├── DCC-100v3说明书-2026-05-24.pdf      # DCC-100v3 说明书
  ├── DCC-100v1v2说明书.pdf               # 旧版说明书
  ├── DCC-100-stm32f103c8t6/              # STM32 参考代码
  └── 14_step_motor_mspm0g3507/            # MSPM0G3507 开环基础旋转
```

每个 CCS 工程的 `Debug/ti_msp_dl_config.h` 是 SysConfig 生成的真实引脚定义，不可手动编辑但可查看。

## 引脚来源

- 小车 TB6612/编码器/灰度全部引脚 → `【小车】03_PID_car/11_PID_car/Debug/ti_msp_dl_config.h`，已整理到 `control-car/hardware/pinmap.md`
- 云台 DCC-100v3 步进电机引脚 → `【云台】08/empty.c` 和 `step_motor.h` 注释，已整理到 `control-car/hardware/gimbal-pinmap.md`
- 不要使用其他来源的引脚号

## 文件应该放哪里

```text
README.md / CLAUDE.md / AGENTS.md    根目录说明
common/                              通用算法、通用硬件、通用测试方法
比赛文档/                            总要求、训练计划、采购原则、官方资料摘要
G-circuit-model/                     G题专用方案、固件、仿真、硬件、报告
control-car/                         小车训练平台、循迹、PID、电机、通信、显示
<new-topic>/                         以后每个新题目都在顶层新建
```

## 处理原则

- 新题目直接在仓库顶层创建文件夹，不再放入 `problems/`。
- 不要再向根目录添加零散方案文档或 HTML。
- 题目专用资料放进对应题目文件夹。
- 通用算法和通用硬件模块放进 `common/`。
- 总要求、总计划、比赛规则摘要放进 `比赛文档/`。
- MSPM0 不要用 ST-LINK 下载。
- `control-car/` 的第一目标是稳定完成 MSPM0G3507 小车底层闭环：PWM 电机控制、编码器测速、灰度循迹、速度 PID、转向 PID、OLED/串口调参。

## CI 门禁

`.github/workflows/repo-structure-gate.yml` 会在 PR 和 push 到 main 时自动触发。核心检查（`scripts/ci/check_repo_structure.py`）：

- 根目录必须存在 `README.md`、`AGENTS.md`、`CLAUDE.md`
- 必须存在 `common/`、`比赛文档/`、`G-circuit-model/`、`control-car/`
- 禁止顶层出现 `problems/`、`docs/`、`stm32/`、`matlab/`、`agent/`
- 每个题目文件夹下必须有 `README.md`
- 如果 PR 违反规则，门禁会报错，阻止合并

## 多人协作

### 工作流

1. 先同步 main：`git checkout main && git pull origin main`
2. 建分支：`git checkout -b <类型>/<描述>` （参考分支命名规范）
3. 修改代码，确保文件放在正确目录
4. 提交：`git add <文件> && git commit -m "类型: 描述"`
5. 推送：`git push -u origin <分支名>`
6. 创建 PR（会自动使用 `.github/PULL_REQUEST_TEMPLATE.md`）
7. 等 CI 门禁通过 + Review 通过
8. 合并到 main

### 分支命名

```
docs/control-car-wiring      文档更新
firmware/control-car-pid     固件代码
hardware/control-car-pinmap  硬件资料
ci/prgate                    CI/脚本修改
fix/readme-links             修复
topic/<新题目>                新题目分支
```

### PR 要求

PR 必须说明：改了什么、为什么改、影响范围、验证方式。从模板勾选影响范围和变更类型。

### CODEOWNERS

配置在 `.github/CODEOWNERS`，默认 owner 为 @37chengshan。根目录治理文件和主要目录都需要 owner 审核。
