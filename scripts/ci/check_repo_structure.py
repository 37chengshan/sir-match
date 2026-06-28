#!/usr/bin/env python3
from __future__ import annotations

from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]

ALLOWED_TOP_LEVEL_FILES = {
    "README.md",
    "CLAUDE.md",
    "AGENTS.md",
    "CONTRIBUTING.md",
    "GOVERNANCE.md",
    "LICENSE",
    ".gitignore",
    ".gitattributes",
}

ALLOWED_TOP_LEVEL_DIRS = {
    "common",
    "比赛文档",
    "G-circuit-model",
    "control-car",
    ".github",
    "scripts",
}

BANNED_TOP_LEVEL_DIRS = {
    "problems",
    "docs",
    "stm32",
    "matlab",
    "agent",
}

TOPIC_DIRS = {
    "G-circuit-model",
    "control-car",
}

ALLOWED_TOPIC_CHILDREN = {
    "README.md",
    "docs",
    "firmware",
    "simulation",
    "hardware",
    "test-data",
    "report",
    "web",
}

REQUIRED_TOPIC_CHILDREN = {
    "README.md",
}

IGNORE_DIR_NAMES = {
    ".git",
    ".venv",
    "node_modules",
    "__pycache__",
}


def fail(errors: list[str]) -> None:
    print("repo structure gate failed:\n")
    for err in errors:
        print(f"- {err}")
    sys.exit(1)


def first_nonempty_line(path: Path) -> str:
    try:
        for line in path.read_text(encoding="utf-8").splitlines():
            if line.strip():
                return line.strip()
    except UnicodeDecodeError:
        return ""
    return ""


def check_top_level(errors: list[str]) -> None:
    for item in ROOT.iterdir():
        name = item.name
        if name in IGNORE_DIR_NAMES:
            continue
        if item.is_dir():
            if name in BANNED_TOP_LEVEL_DIRS:
                errors.append(f"banned top-level directory found: {name}/")
            elif name not in ALLOWED_TOP_LEVEL_DIRS:
                errors.append(
                    f"unknown top-level directory: {name}/. "
                    "Add it to README, AGENTS, CLAUDE, GOVERNANCE and this gate script in the same PR."
                )
        else:
            if name not in ALLOWED_TOP_LEVEL_FILES:
                errors.append(f"unknown top-level file: {name}")


def check_topic_dirs(errors: list[str]) -> None:
    for topic in sorted(TOPIC_DIRS):
        topic_path = ROOT / topic
        if not topic_path.exists():
            errors.append(f"missing topic directory: {topic}/")
            continue
        for required in REQUIRED_TOPIC_CHILDREN:
            if not (topic_path / required).exists():
                errors.append(f"{topic}/ is missing required item: {required}")
        for child in topic_path.iterdir():
            if child.name in IGNORE_DIR_NAMES:
                continue
            if child.name not in ALLOWED_TOPIC_CHILDREN:
                errors.append(f"unexpected item in {topic}/: {child.name}")


def check_markdown_headings(errors: list[str]) -> None:
    for path in ROOT.rglob("*.md"):
        if any(part in IGNORE_DIR_NAMES for part in path.parts):
            continue
        rel = path.relative_to(ROOT)
        first = first_nonempty_line(path)
        if not first.startswith("# "):
            errors.append(f"markdown file must start with a level-1 heading: {rel}")


def main() -> None:
    errors: list[str] = []
    check_top_level(errors)
    check_topic_dirs(errors)
    check_markdown_headings(errors)
    if errors:
        fail(errors)
    print("repo structure gate passed")


if __name__ == "__main__":
    main()
