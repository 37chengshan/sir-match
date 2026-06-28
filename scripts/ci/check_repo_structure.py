#!/usr/bin/env python3
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[2]
errors = []

for name in ["README.md", "AGENTS.md", "CLAUDE.md"]:
    if not (root / name).is_file():
        errors.append("missing file: " + name)

for name in ["common", "比赛文档", "G-circuit-model", "control-car"]:
    if not (root / name).is_dir():
        errors.append("missing directory: " + name)

for name in ["problems", "docs", "stm32", "matlab", "agent"]:
    if (root / name).exists():
        errors.append("unexpected top-level item: " + name)

for name in ["G-circuit-model", "control-car"]:
    if (root / name).is_dir() and not (root / name / "README.md").is_file():
        errors.append("missing topic README: " + name)

if errors:
    print("repo-structure-gate failed")
    for item in errors:
        print("- " + item)
    sys.exit(1)

print("repo-structure-gate passed")
