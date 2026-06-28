# 仓库治理规则

本仓库按多人协作方式管理。重要修改应通过 Pull Request 合并，避免直接改 main。

## 1. 核心原则

1. `main` 只保留可用内容。
2. 新工作先建分支，再开 PR。
3. 题目文件夹放在仓库顶层。
4. 通用内容放在 `common/`。
5. 总计划和比赛资料放在 `比赛文档/`。
6. 自动检查脚本放在 `scripts/ci/`。

## 2. 允许的主要顶层目录

```text
common/
比赛文档/
G-circuit-model/
control-car/
.github/
scripts/
```

## 3. 禁止重新使用的旧目录

```text
problems/
docs/
stm32/
matlab/
agent/
```

## 4. PR 要求

每个 PR 至少说明：

1. 改了什么；
2. 为什么改；
3. 影响范围；
4. 验证方式。

## 5. 评审要求

正式协作时建议启用：

```text
Require a pull request before merging
Required approvals: 1
Require review from Code Owners
Require conversation resolution before merging
Block force pushes
Restrict deletions
Require status checks to pass
```

其中 required status check 选择：

```text
repo-structure-gate
```
