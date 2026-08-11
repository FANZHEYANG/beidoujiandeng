# RDSS LF Fallback Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Convert an invalid App-provided `lf` value to DM229's recommended value `3` when constructing `$CCMSG`.

**Architecture:** Keep the BLE 0x4504 parser and `Msg_tx` unchanged. Normalize `Msg_tx.lf` locally inside the RDSS transmit event, use the normalized byte in `$CCMSG`, and expose both values through the selected send-test log.

**Tech Stack:** C99 firmware, TMOS event processing, Python source-regression checks

---

### Task 1: Normalize the DM229 frequency value

**Files:**
- Create: `tools/check_rdss_lf_fallback.py`
- Modify: `HAL/decode.c:959-1008`

- [x] **Step 1: Write the failing regression check**

Create a source check that requires a local effective LF variable, accepts only `2`, `3`, and `5`, falls back to `3`, uses the effective value in `sprintf(strCCMSG, ...)`, and logs both the App and effective values.

- [x] **Step 2: Run the check and verify it fails**

Run:

```powershell
python tools/check_rdss_lf_fallback.py
```

Expected: `FAIL` because `HAL/decode.c` currently passes `Msg_tx.lf` directly to `$CCMSG`.

- [x] **Step 3: Implement the minimal normalization**

Add a local byte initialized from `Msg_tx.lf`, then normalize it before command construction:

```c
uint8_t rdss_lf = Msg_tx.lf;

if((rdss_lf != 2) && (rdss_lf != 3) && (rdss_lf != 5))
{
    rdss_lf = 3;
}
```

Use `rdss_lf` in the `$CCMSG` `sprintf` call. Update the selected transmit log to include `app_lf=Msg_tx.lf` and `tx_lf=rdss_lf`.

- [x] **Step 4: Run focused and existing checks**

Run:

```powershell
python tools/check_rdss_lf_fallback.py
python tools/check_rdss_send_test_prints.py
python tools/check_gnss_prints.py
python -m py_compile tools/check_rdss_lf_fallback.py tools/check_rdss_send_test_prints.py tools/check_gnss_prints.py
```

Expected: every command exits with status `0` and every source check reports `PASS`.

- [x] **Step 5: Review and commit only scoped files**

Run `git diff --check`, inspect the diff, and commit `HAL/decode.c`, the new regression check, and this plan without staging the unrelated `HAL/PWR.c` change.
