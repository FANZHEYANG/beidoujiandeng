# RDSS LF Fallback Design

## Goal

Keep the existing App 0x4504 payload unchanged while preventing an invalid
`lf` value from producing a DM229 `$CCMSG` command that is rejected with
`BDFKI reason=12`.

## Behavior

- Preserve the `lf` value received from the App in `Msg_tx.lf`.
- When building `$CCMSG`, accept DM229 frequency values `2`, `3`, and `5`.
- Map every other value, including the App's current fixed value `1`, to the
  recommended DM229 frequency value `3`.
- Use the normalized value both in the command and its XOR checksum.
- Include the App value and effective DM229 value in the selected RDSS send
  test log.

## Scope

The change is limited to the RDSS transmit command construction in
`HAL/decode.c`. It does not change the App, the 0x4504 BLE layout, payload
encoding, destination-card parsing, acknowledgement parsing, or RNSS/RDSS
power sequencing.

## Verification

A static regression check will verify that invalid values fall back to `3`,
valid values remain unchanged, and `$CCMSG` uses the normalized value. The
existing RDSS send-print and GNSS-print checks must continue to pass.
