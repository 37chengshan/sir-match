#!/usr/bin/env python3
from __future__ import annotations

import os
import sys

REQUIRED_SECTIONS = [
    "## 1. 本次改了什么",
    "## 2. 为什么要改",
    "## 3. 影响范围",
    "## 4. 变更类型",
    "## 5. 验证方式",
    "## 7. 自检清单",
]

REQUIRED_CHECKBOX_MARKERS = [
    "- [ ]",
]


def main() -> None:
    body = os.environ.get("PR_BODY", "") or ""
    errors: list[str] = []

    for section in REQUIRED_SECTIONS:
        if section not in body:
            errors.append(f"PR body is missing required section: {section}")

    if not any(marker in body for marker in REQUIRED_CHECKBOX_MARKERS):
        errors.append("PR body must include checklist items")

    if errors:
        print("PR body gate failed:\n")
        for err in errors:
            print(f"- {err}")
        print("\nUse .github/PULL_REQUEST_TEMPLATE.md to fill the PR description.")
        sys.exit(1)

    print("PR body gate passed")


if __name__ == "__main__":
    main()
