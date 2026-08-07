# Disable GNSS Debug Print Design

## Goal

Keep GNSS reception, NMEA parsing, positioning, SNR processing, voice events,
and BLE notifications active while removing GNSS serial debug noise so water
voltage diagnostics remain visible.

## Design

- Add `GNSS_DEBUG_PRINT_ENABLE` in `HAL/decode.c`, defaulted to `0`.
- Route only GNSS parser, GSV, GGA, receive-statistics, warning, and raw-byte
  diagnostics through `GNSS_DEBUG_PRINT(...)`.
- Make the debug-output UART receive mute a no-op while GNSS logging is off,
  so disabling logs cannot discard incoming NMEA bytes.
- Keep RDSS transmit, receive, acknowledgement, and card diagnostics on the
  existing `PRINT(...)` path.
- Do not change GNSS task scheduling, parsing results, or BLE notification data.
- Preserve the user's unrelated `HAL/PWR.c` water-threshold change.

## Verification

- A focused static check requires the GNSS print switch to be disabled and
  verifies GNSS diagnostics use the dedicated macro.
- The same check verifies representative RDSS diagnostics still use `PRINT`.
- Attempt the firmware build and report toolchain availability separately.
