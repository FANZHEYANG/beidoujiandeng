# RDSS Acknowledgement Restore Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Restore RNSS, audio, and LEDs only after a DM229 message acknowledgement or a bounded timeout.

**Architecture:** Add pending-send, restore-delay, timeout, and late-ack cooldown states to the existing RDSS TMOS task. Move output ownership into focused PWR suspend/restore functions so SOS and water flash state can be preserved without exposing private flags.

**Tech Stack:** C99 firmware, TMOS events, Python source-regression checks

---

### Task 1: Add the failing timing regression check

**Files:**
- Create: `tools/check_rdss_ack_restore.py`

- [x] **Step 1: Require pending-send state, MSG acknowledgement gating, restore delay, timeout, and PWR output ownership.**
- [x] **Step 2: Run `python tools/check_rdss_ack_restore.py` and verify it fails because the current code reopens RNSS immediately.**

### Task 2: Implement acknowledgement and timeout restoration

**Files:**
- Modify: `HAL/include/decode.h`
- Modify: `HAL/decode.c:959-1312`

- [x] **Step 1: Add free RDSS event bits for delayed restore and send timeout.**
- [x] **Step 2: Mark a send pending after UART transmission, reject pre-send queued acknowledgements by frame sequence, and start a 15-second timeout instead of opening RNSS.**
- [x] **Step 3: On a valid `MSG` `$BDFKI`, cancel timeout and schedule restore after 800 ms.**
- [x] **Step 4: Restore on timeout or timer-allocation failure, filter late acknowledgements for 5 seconds after timeout, and permit the next queued send only after cooldown.**

### Task 3: Preserve audio and LED behavior

**Files:**
- Modify: `HAL/include/PWR.h`
- Modify: `HAL/PWR.c`

- [x] **Step 1: Add PWR suspend/restore functions that snapshot LED state and close audio.**
- [x] **Step 2: Prevent periodic flash updates and voice dequeue while RDSS output suspension is active.**
- [x] **Step 3: Restore current flashing or the saved static state, while respecting soft power-off.**

### Task 4: Verify and publish

**Files:**
- Modify: `docs/superpowers/plans/2026-08-11-rdss-ack-restore.md`

- [x] **Step 1: Run the new check, existing RDSS/GNSS/BLE checks, Python syntax checks, and `git diff --check`.**
- [x] **Step 2: Review the scoped diff and obtain an independent read-only code review.**
- [ ] **Step 3: Commit only this feature's files, leave the existing `HAL/PWR.c` 500 mV user change intact in the same file, and push `main`.**
