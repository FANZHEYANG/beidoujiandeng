# RDSS Send Test Print Mode Design

## Goal

Provide a temporary serial-log mode that shows the complete APP-to-DM229 data
send and self-receive path while suppressing unrelated firmware diagnostics.

## Included Logs

- APP `0x4504` write validation, parsed fields, and payload bytes.
- DM229 `$CCMSG` destination, encoding, payload length, and complete command.
- DM229 `$BDFKI` acknowledgement result.
- APP `0x4505` acknowledgement notification result.
- DM229 `$BDMXX` self-received message fields and payload bytes.
- APP `0x4506` received-message notification result.

## Suppressed Logs

- GNSS/GGA/GSV diagnostics.
- Water-voltage, SOS, location-report, battery, and voice diagnostics.
- Bluetooth connection, advertising, RSSI, profile, card, and SNR diagnostics.
- MCU, RTC, and other general `PRINT` or `printf` output.

## Architecture

- Add `RDSS_SEND_TEST_ONLY_PRINT`, enabled by default for the current test, and
  disable `SOS_TEST_ONLY_PRINT` in `HAL/include/CONFIG.h`.
- In `HAL/include/HAL.h`, map `RDSS_SEND_TEST_PRINT(...)` directly to the
  underlying `printf` function, then suppress ordinary `PRINT(...)` and
  `printf(...)` calls while the mode is enabled.
- Route only the selected 4504, transmit, acknowledgement, self-receive, 4505,
  and 4506 diagnostics through `RDSS_SEND_TEST_PRINT(...)`.
- Do not change packet parsing, UART transmission, BLE notification behavior,
  satellite power sequencing, or the user's uncommitted water threshold.

## Verification

- A focused source check verifies the mode switches, global suppression, and
  all required send-path labels.
- The check rejects selected send-path labels that still use general print
  macros and verifies representative unrelated logs remain on suppressed paths.

