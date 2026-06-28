# 仓库协作治理规则

本仓库从个人备赛仓库升级为多人协作仓库。核心目标是：任何重要变更都有来源、有审查、有目录归属、有验证结果，避免多人同时写乱结构。

## 1. 总原则

1. `main` 分支只保留可用、可读、可追溯的内容。
2. 所有成员通过 Pull Request 合并，不直接向 `main` 提交。
3. 每个题目必须是仓库顶层文件夹，例如 `G-circuit-model/`、`control-car/`。
4. 通用算法、通用硬件、通用测试方法放入 `common/`。
5. 比赛总计划、总要求、官方资料摘要放入 `比赛文档/`。
6. 根目录只允许保留仓库级说明与协作文件。
7. 新增题目文件夹、修改目录结构、修改 CI 门禁，必须经过 PR 审查。

## 2. 分支规则

推荐分支命名：

| 类型 | 命名格式 | 示例 |
|---|---|---|
| 文档 | `docs/<scope>-<short-name>` | `docs/control-car-wiring` |
| 固件 | `firmware/<topic>-<short-name>` | `firmware/control-car-pid` |
| 硬件 | `hardware/<topic>-<short-name>` | `hardware/control-car-pinmap` |
| 仿真 | `simulation/<topic>-<short-name>` | `simulation/g-circuit-goertzel` |
| 治理 | `governance/<short-name>` | `governance/pr-gates` |
| 修复 | `fix/<scope>-<short-name>` | `fix/readme-links` |

不推荐：

```text
main2
test
new
final
mywork
```

## 3. PR 必须包含什么

每个 PR 必须说明：

1. 本次改了什么；
2. 为什么要改；
3. 影响哪些目录；
4. 是否改了硬件接线、引脚、供电；
5. 是否改了固件接口、算法、通信协议；
6. 如何验证；
7. 是否需要其他人复测。

PR 模板位于：

```text
.github/PULL_REQUEST_TEMPLATE.md
```

## 4. 目录门禁

仓库只允许以下顶层入口：

```text
README.md
CLAUDE.md
AGENTS.md
CONTRIBUTING.md
GOVERNANCE.md
common/
比赛文档/
G-circuit-model/
control-car/
.github/
scripts/
```

禁止重新引入：

```text
problems/
docs/
stm32/
matlab/
agent/
```

如确实需要新增顶层题目文件夹，必须在同一个 PR 中同时更新：

1. `README.md`；
2. `AGENTS.md`；
3. `CLAUDE.md`；
4. `scripts/ci/check_repo_structure.py`；
5. 新题目文件夹下的 `README.md`。

## 5. 题目文件夹标准结构

每个题目文件夹使用统一结构：

```text
<topic>/
├── README.md
├── docs/
├── firmware/
├── simulation/
├── hardware/
├── test-data/
├── report/
└── web/
```

允许暂时没有真实代码，但必须有说明文件，不允许只丢一堆零散文件。

## 6. 评审规则

### 6.1 文档变更

文档 PR 至少需要检查：

- 标题清楚；
- 有结论；
- 有适用范围；
- 文件位置正确；
- 与 README/AGENTS/CLAUDE 不冲突。

### 6.2 硬件变更

硬件 PR 必须检查：

- pinmap 是否来自原理图、焊接图或实测；
- 电源电压是否写清楚；
- 是否说明共地；
- 是否说明风险点；
- 是否有上电检查步骤。

### 6.3 固件变更

固件 PR 必须检查：

- 是否有模块边界；
- 是否避免把所有逻辑塞进 `main.c`；
- 是否有状态机或清晰流程；
- 是否有参数限幅；
- 是否有调试输出；
- 是否写明如何编译或测试。

### 6.4 算法变更

算法 PR 必须检查：

- 是否平台无关；
- 输入输出是否明确；
- 是否说明单位；
- 是否说明适用场景；
- 是否给出最小测试方式。

## 7. CODEOWNERS

目录负责人由 `.github/CODEOWNERS` 管理。当前默认 owner 是 `@37chengshan`。后续多人加入后，把对应 GitHub 用户名加入相应目录即可。

建议后续分工：

| 目录 | 负责人类型 |
|---|---|
| `common/` | 算法/硬件公共模块负责人 |
| `G-circuit-model/` | G 题负责人 |
| `control-car/` | 小车控制负责人 |
| `比赛文档/` | 资料和计划负责人 |
| `.github/`、`scripts/` | 仓库治理负责人 |

## 8. GitHub 设置建议

需要在 GitHub 网页端手动开启：

1. Settings → Branches → Branch protection rules；
2. 保护分支：`main`；
3. 勾选 `Require a pull request before merging`；
4. 勾选 `Require approvals`，建议至少 1 个 approval；
5. 勾选 `Require review from Code Owners`；
6. 勾选 `Require status checks to pass before merging`；
7. 选择工作流检查：`repo-structure-gate`；
8. 勾选 `Require branches to be up to date before merging`；
9. 如团队成熟，可勾选 `Do not allow bypassing the above settings`。

仅添加 CODEOWNERS 和 CI 文件还不能真正阻止直接 push，必须开启分支保护。
