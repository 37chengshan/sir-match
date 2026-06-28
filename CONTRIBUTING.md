# 协作提交指南

本仓库采用 Pull Request 协作流程。任何成员修改仓库内容，都应先建分支，再开 PR，经检查通过后合并。

## 1. 标准流程

```text
同步 main
→ 新建功能分支
→ 修改文件
→ 本地自查
→ 提交 commit
→ push 分支
→ 创建 Pull Request
→ 等待 CI 和 review
→ 修改问题
→ 合并 PR
```

## 2. 开始一个任务

先确认任务属于哪个范围：

| 任务 | 放置位置 |
|---|---|
| G 题专用 | `G-circuit-model/` |
| 小车专用 | `control-car/` |
| 通用算法 | `common/algorithms/` |
| 通用硬件 | `common/hardware/` |
| 总计划和总要求 | `比赛文档/` |
| 仓库治理 | `.github/`、`scripts/`、`GOVERNANCE.md` |

## 3. 分支命名

```text
docs/control-car-wiring
firmware/control-car-pid
hardware/control-car-pinmap
simulation/g-circuit-model
fix/readme-structure
governance/pr-gates
```

## 4. Commit 信息

推荐格式：

```text
<type>: <short summary>
```

常用 type：

| type | 用途 |
|---|---|
| `docs` | 文档 |
| `firmware` | 固件 |
| `hardware` | 硬件资料 |
| `algorithm` | 算法 |
| `test` | 测试数据或测试说明 |
| `governance` | 仓库治理、CI、PR 规则 |
| `fix` | 修复 |

示例：

```text
docs: add control car wiring guide
firmware: add PID controller skeleton
hardware: update MSPM0 car pinmap
governance: add PR gate workflow
```

## 5. PR 自查清单

提交 PR 前确认：

- [ ] 文件放在正确目录；
- [ ] 没有新增禁止的顶层目录；
- [ ] 文档有一级标题；
- [ ] 说明了为什么改；
- [ ] 说明了怎么验证；
- [ ] 硬件相关内容写清电压、引脚和风险；
- [ ] 固件相关内容写清模块边界和测试方法；
- [ ] 不和 `README.md`、`AGENTS.md`、`CLAUDE.md` 的结构规则冲突。

## 6. 合并前要求

PR 合并前应满足：

1. CI 通过；
2. 至少 1 人 review；
3. 涉及目录负责人已确认；
4. 文档与代码没有明显冲突；
5. 新增题目文件夹已补齐标准结构说明。
