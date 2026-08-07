# Disable GNSS Debug Print Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Silence GNSS serial diagnostics without stopping GNSS processing or unrelated debug output.

**Architecture:** Add a compile-time GNSS-only logging macro in `HAL/decode.c`. Replace active GNSS diagnostic calls with that macro while leaving UART input, parsing, BLE data flow, and RDSS logging unchanged.

**Tech Stack:** CH58x C firmware and a focused Python source regression check.

---

### Task 1: Add a failing GNSS print regression check

**Files:**
- Create: `tools/check_gnss_prints.py`

- [x] Require `GNSS_DEBUG_PRINT_ENABLE` to equal `0`.
- [x] Require GSV, GGA, RX statistics, and raw-byte diagnostics to use `GNSS_DEBUG_PRINT`.
- [x] Require representative RDSS diagnostics to remain on `PRINT`.
- [x] Run `python tools/check_gnss_prints.py` and confirm it fails on the existing direct GNSS `PRINT` calls.

### Task 2: Route GNSS diagnostics through the disabled macro

**Files:**
- Modify: `HAL/decode.c`

- [x] Define `GNSS_DEBUG_PRINT(...)` as `PRINT(...)` when enabled and empty when disabled.
- [x] Replace active GNSS parser, GSV, GGA, receive-statistics, warning, and raw diagnostics.
- [x] Disable the print-related UART receive mute when GNSS logging is disabled.
- [x] Leave RDSS diagnostics and GNSS data processing unchanged.

### Task 3: Verify and archive

**Files:**
- Test: `tools/check_gnss_prints.py`
- Build: `Peripheral/obj/makefile`

- [x] Run `python tools/check_gnss_prints.py` and require a zero exit code.
- [x] Run `python -m py_compile tools/check_gnss_prints.py`.
- [x] Run `git diff --check` and inspect the scoped diff.
- [x] Attempt `make all` from `Peripheral/obj`.
- [ ] Commit only the GNSS print files and documentation, then push `main`.
