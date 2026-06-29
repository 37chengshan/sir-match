# PR Gate Smoke Test

This file is intentionally small. It is used to open a test pull request so GitHub Actions can discover and run the repository structure gate.

Expected result:

- The repository structure gate should pass.
- The PR body gate should pass.
- Branch protection should still require review before merge.
