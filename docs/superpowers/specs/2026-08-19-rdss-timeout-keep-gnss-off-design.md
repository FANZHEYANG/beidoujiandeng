# RDSS Timeout Keep GNSS Off Design

## Goal

After a 4504 RDSS transmission times out without a matching `MSG BDFKI`, keep
GNSS powered off. A matching `MSG BDFKI` must continue to restore GNSS after
the existing delay.

## Design

The RDSS restore scheduler will carry an explicit `restore_gnss` flag across
the delayed restore event. The acknowledgement path schedules restore with the
flag set, while the timeout and timeout-timer-failure paths clear it.

`Rdss_RestoreNow()` will always restore the temporarily suspended LED and audio
outputs. It will call `OPENRN()` and set `RN_SW_Flag` only when `restore_gnss`
is set and the board is not in software power-off. The existing five-second
late-ack guard after a timeout remains unchanged, and RDSS power remains on.

## Observable behavior

- Matching `MSG BDFKI`: log ends with `[RDSS TX RESTORE] complete rn=1` and
  GNSS restarts.
- No matching `MSG BDFKI` before timeout: log ends with
  `[RDSS TX RESTORE] complete rn=0`; GNSS stays off and the late-ack guard runs.
- LED/audio restoration, BLE state, RDSS power, and queued-send handling are
  unchanged.

## Verification

Extend `tools/check_rdss_ack_restore.py` first so the current implementation
fails the new assertions. Then implement the flag propagation and rerun the
check. Finally run the other RDSS source checks and the available project build
command if one is present.
