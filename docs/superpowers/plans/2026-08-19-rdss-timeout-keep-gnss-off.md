# RDSS Timeout Keep GNSS Off Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Keep GNSS off after an RDSS 4504 acknowledgement timeout while preserving GNSS restoration after a matching BDFKI acknowledgement.

**Architecture:** Store an explicit restore-GNSS decision when scheduling the delayed RDSS restore. Consume and clear that decision in `Rdss_RestoreNow()` so acknowledgement and timeout paths can share the output-restoration state machine without sharing GNSS behavior.

**Tech Stack:** CH584 embedded C, TMOS events, Python source-invariant checks.

---

### Task 1: Add the failing timeout restore check

**Files:**
- Modify: `tools/check_rdss_ack_restore.py`

- [ ] **Step 1: Assert the required restore decisions**

Add assertions requiring an explicit `rdss_restore_gnss` state variable, an
acknowledgement call that passes `restore_gnss=1`, and timeout calls that pass
`restore_gnss=0`:

```python
assert "static uint8_t rdss_restore_gnss = 0;" in decode_c
assert 'Rdss_ScheduleRestore(RDSS_RN_RESTORE_DELAY, "ack", 0, 1);' in ack_tail
assert 'Rdss_ScheduleRestore(1, "timeout", 1, 0);' in rdss_process
assert 'Rdss_ScheduleRestore(1, "timeout_timer_fail", 1, 0);' in send_block
```

- [ ] **Step 2: Run the check and verify RED**

Run:

```powershell
python tools/check_rdss_ack_restore.py
```

Expected: `AssertionError` because the restore decision is not implemented.

### Task 2: Propagate the GNSS restore decision

**Files:**
- Modify: `HAL/decode.c`

- [ ] **Step 1: Add and consume the decision flag**

Add:

```c
static uint8_t rdss_restore_gnss = 0;
```

At the start of `Rdss_RestoreNow()`, copy the flag to a local variable and
clear the stored value. Restore GNSS only when the copied value is nonzero:

```c
uint8_t restore_gnss = rdss_restore_gnss;
rdss_restore_gnss = 0;

if((restore_gnss != 0) && (Pwr_IsSoftPowerOff() == FALSE))
{
    RN_SW_Flag = TRUE;
    OPENRN();
}
```

- [ ] **Step 2: Extend the scheduler and its callers**

Change the scheduler to accept and store the decision:

```c
static void Rdss_ScheduleRestore(uint16_t delay_ticks, const char *source,
                                 uint8_t start_cooldown,
                                 uint8_t restore_gnss)
{
    /* existing state guard */
    rdss_restore_gnss = restore_gnss;
    /* existing scheduling logic */
}
```

Use these exact call decisions:

```c
Rdss_ScheduleRestore(RDSS_RN_RESTORE_DELAY, "ack", 0, 1);
Rdss_ScheduleRestore(1, "timeout", 1, 0);
Rdss_ScheduleRestore(1, "timeout_timer_fail", 1, 0);
```

- [ ] **Step 3: Run the focused check and verify GREEN**

Run:

```powershell
python tools/check_rdss_ack_restore.py
```

Expected: `PASS: outputs restore after BDFKI or timeout; GNSS restores only after BDFKI`.

### Task 3: Regression verification

**Files:**
- Verify only

- [ ] **Step 1: Run all RDSS checks**

Run:

```powershell
Get-ChildItem tools/check_rdss_*.py | ForEach-Object { python $_.FullName }
```

Expected: every script exits with code zero.

- [ ] **Step 2: Review the scoped diff**

Run:

```powershell
git diff -- HAL/decode.c tools/check_rdss_ack_restore.py docs/superpowers/specs/2026-08-19-rdss-timeout-keep-gnss-off-design.md docs/superpowers/plans/2026-08-19-rdss-timeout-keep-gnss-off.md
```

Expected: only the restore decision, its checks, and documentation are added;
the existing commented or uncommented RDSS UART send line is untouched.
