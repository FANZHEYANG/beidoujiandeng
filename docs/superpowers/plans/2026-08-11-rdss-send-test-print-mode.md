# RDSS Send Test Print Mode Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Show only the complete RDSS data-send and self-receive serial diagnostics during board testing.

**Architecture:** Add a global test-mode switch and a dedicated `RDSS_SEND_TEST_PRINT` macro. Suppress ordinary prints centrally in `HAL.h`, then route only selected 4504, DM229 TX/RX/ACK, 4505, and 4506 messages through the dedicated macro.

**Tech Stack:** CH58x C firmware and Python source regression checks.

---

### Task 1: Add A Failing Print-Mode Regression Check

**Files:**
- Create: `tools/check_rdss_send_test_prints.py`

- [ ] Require `RDSS_SEND_TEST_ONLY_PRINT` to be enabled and
  `SOS_TEST_ONLY_PRINT` to be disabled.
- [ ] Require `HAL.h` to suppress ordinary prints and expose the dedicated send
  test macro.
- [ ] Require 4504, `$CCMSG`, `$BDFKI`, 4505, `$BDMXX`, and 4506 diagnostics to
  use the dedicated macro.
- [ ] Run `python tools/check_rdss_send_test_prints.py` and confirm it fails
  because the dedicated mode does not exist.

### Task 2: Add The Global Send-Test Print Mode

**Files:**
- Modify: `HAL/include/CONFIG.h`
- Modify: `HAL/include/HAL.h`

- [ ] Add and enable `RDSS_SEND_TEST_ONLY_PRINT`.
- [ ] Disable `SOS_TEST_ONLY_PRINT`.
- [ ] Define `RDSS_SEND_TEST_PRINT(...)` before replacing ordinary `PRINT` and
  `printf` with empty macros.

### Task 3: Route The Complete Send Path

**Files:**
- Modify: `Peripheral/Profile/gattprofile.c`
- Modify: `HAL/decode.c`
- Modify: `Peripheral/APP/peripheral.c`

- [ ] Route all 4504 write success and failure diagnostics through the dedicated
  macro.
- [ ] Route `$CCMSG`, `$BDFKI`, and `$BDMXX` diagnostics through the dedicated
  macro.
- [ ] Route 4505 diagnostics through the dedicated macro and add 4506 success,
  failure, allocation, and MTU diagnostics.
- [ ] Run the focused check, existing focused checks, Python syntax checks, and
  `git diff --check`.
- [ ] Commit only this feature's files and push `main`.

