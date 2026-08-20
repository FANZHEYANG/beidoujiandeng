# RDSS SNR Auto-Send Test Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Automatically queue one GBK `测试` message to card `4209606` after DM229 reports at least three SNR channels at or above 48 and the send countdown reaches zero.

**Architecture:** Add a compile-time test switch and a focused helper in `HAL/decode.c`. The helper fills the existing `Msg_tx`/`DestIC` interface and wakes the current RDSS TMOS task, preserving the normal UART command, BDFKI acknowledgement, timeout, cooldown, and output-restore state machine.

**Tech Stack:** CH584 C firmware, DM229 `$BDSNR` parsing, TMOS events, Python source-regression checks

---

### Task 1: Add the failing source regression check

**Files:**
- Create: `tools/check_rdss_snr_auto_send_test.py`

- [x] **Step 1: Require the test switch, one-shot helper, exact threshold, fixed destination, GBK payload, valid countdown, idle guards, and existing RDSS task wake-up.**

The check reads `HAL/include/CONFIG.h` and `HAL/decode.c`, extracts the helper and `$BDSNR` branch, then asserts the exact agreed behavior.

- [x] **Step 2: Run `python tools/check_rdss_snr_auto_send_test.py`.**

Expected: FAIL because `RDSS_SNR_AUTO_SEND_TEST` and `Rdss_TryQueueSnrAutoSendTest()` do not exist yet.

### Task 2: Implement one-shot SNR-triggered test queueing

**Files:**
- Modify: `HAL/include/CONFIG.h`
- Modify: `HAL/decode.c`

- [x] **Step 1: Add `RDSS_SNR_AUTO_SEND_TEST` with a default value of `TRUE`.**
- [x] **Step 2: Add a static one-shot flag and `Rdss_TryQueueSnrAutoSendTest()`.**

The helper must return unless all conditions are true: switch enabled, not previously triggered, a valid `$BDTTC` has been parsed, `snr[0..2] >= 48`, `frequency_count_down == 0`, `RD_txflag == 0`, RDSS send state is idle, and soft power-off is false.

- [x] **Step 3: Fill the existing send structure with `lf=3`, `encode=2`, `generation=3`, destination `4209606`, zero health fields, and payload bytes `{0xB2, 0xE2, 0xCA, 0xD4}`.**
- [x] **Step 4: Mark the one-shot flag before queueing, power RDSS, set `RD_txflag`, and call `tmos_set_event(RDSS_TaskID, rdss_evt)`.**
- [x] **Step 5: Call the helper only after the `$BDSNR` branch has successfully parsed and sorted all signal values.**

### Task 3: Verify the test mode and existing behavior

**Files:**
- Modify: `docs/superpowers/plans/2026-08-13-rdss-snr-auto-send-test.md`

- [x] **Step 1: Run the new source regression check and confirm it passes.**
- [x] **Step 2: Run RDSS acknowledgement, LF fallback, send-print, GNSS-print, BLE-power, and voice-queue checks.**
- [x] **Step 3: Run Python syntax checks and `git diff --check`.**
- [x] **Step 4: Review the scoped diff and document that target compilation requires the unavailable WCH/MounRiver RISC-V toolchain.**
- [ ] **Step 5: Commit and push only when Git write approval is available, preserving unrelated working-tree changes.**
