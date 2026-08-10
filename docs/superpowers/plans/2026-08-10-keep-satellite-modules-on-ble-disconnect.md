# Keep Satellite Modules On BLE Disconnect Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Keep RNSS and RDSS powered after an ordinary Bluetooth link disconnect while preserving explicit shutdown behavior.

**Architecture:** Change only `Peripheral_LinkTerminated()` so it no longer disables the satellite modules. Add a focused Python source check that distinguishes ordinary link termination from explicit Bluetooth shutdown.

**Tech Stack:** CH58x C firmware and Python source regression checks.

---

### Task 1: Add The Disconnect Power Regression Check

**Files:**
- Create: `tools/check_ble_disconnect_satellite_power.py`

- [ ] Create a source check that requires link establishment to enable modules,
  ordinary link termination not to disable modules, and `Peripheral_BleOff()` to
  retain its explicit disable call.
- [ ] Run `python tools/check_ble_disconnect_satellite_power.py` and confirm it
  fails because `Peripheral_LinkTerminated()` currently disables both modules.

### Task 2: Keep Modules On Across Link Termination

**Files:**
- Modify: `Peripheral/APP/peripheral.c`

- [ ] Remove only `peripheralSetSatelliteModules(FALSE)` from
  `Peripheral_LinkTerminated()` and leave connection cleanup and advertising
  restart intact.
- [ ] Run `python tools/check_ble_disconnect_satellite_power.py` and require a
  zero exit code.
- [ ] Run existing focused source checks and `git diff --check`.
- [ ] Commit only this feature's files and push `main`.

