# 协作提交指南

## 标准流程

先同步 main，再新建分支。完成修改后提交 commit，创建 Pull Request，等待检查和 review，通过后再合并。

## 分支命名

推荐格式：

```text
docs/control-car-wiring
firmware/control-car-pid
hardware/control-car-pinmap
ci/prgate
fix/readme-links
```

## 文件放置

通用算法放入 `common/algorithms/`。

通用硬件放入 `common/hardware/`。

比赛资料放入 `比赛文档/`。

G 题内容放入 `G-circuit-model/`。

小车内容放入 `control-car/`。

CI 门禁放入 `.github/` 和 `scripts/ci/`。

## PR 自查

提交前确认文件位置正确，没有新增禁止的顶层目录，文档有标题，并写清验证方式。
