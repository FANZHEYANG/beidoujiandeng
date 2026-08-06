# PA12 Water ADC Range Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make PA12 report its real 0-3.3 V water-detection voltage without ADC saturation.

**Architecture:** Keep the shared ADC and existing water state machine. Configure ADC2 with 1/4 gain for water samples, maintain a water-specific rough calibration, and use the CH58x SDK voltage conversion while restoring the battery ADC configuration in the battery path.

**Tech Stack:** CH58x C firmware, WCH StdPeriphDriver ADC API, Python static regression checks.

---

### Task 1: Add a failing ADC range regression check

**Files:**
- Modify: `tools/check_voice_queue.py`

- [ ] Require `ADC_PGA_1_4`, `WaterRoughCalib_Value`, and `ADC_VoltConverSignalPGA_MINUS_12dB` in the water path.
- [ ] Reject the obsolete proportional conversion constants.
- [ ] Run `python tools/check_voice_queue.py` and confirm it fails because the old implementation still uses `ADC_PGA_1_2`.

### Task 2: Correct the PA12 ADC acquisition

**Files:**
- Modify: `HAL/PWR.c`

- [ ] Add a signed water-only calibration variable.
- [ ] Configure `ADC_PGA_1_4` and ADC2 before water calibration and sampling.
- [ ] Apply the water calibration to PA12 samples.
- [ ] Convert samples with `ADC_VoltConverSignalPGA_MINUS_12dB` and clamp negative millivolts to zero.
- [ ] Preserve the 2000 mV threshold, debounce, and battery ADC configuration.

### Task 3: Verify and archive

**Files:**
- Test: `tools/check_voice_queue.py`
- Build: `Peripheral/obj/Makefile`

- [ ] Run `python tools/check_voice_queue.py` and require a zero exit code.
- [ ] Run `make all` from `Peripheral/obj` and record whether the local toolchain is available.
- [ ] Inspect `git diff --check` and the scoped diff.
- [ ] Commit the files and push `main` to GitHub.
