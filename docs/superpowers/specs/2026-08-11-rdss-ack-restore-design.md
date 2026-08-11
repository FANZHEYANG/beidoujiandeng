# RDSS Acknowledgement Restore Design

## Goal

Keep RNSS, audio, and LEDs suspended during an active DM229 transmission,
then restore them after the matching message acknowledgement or a timeout.

## Send State

- A 0x4504 send closes RNSS and suspends audio and LED output before writing
  `$CCMSG` to DM229.
- Writing the UART command enters a single pending-send state. RNSS is not
  reopened at this point.
- A `$BDFKI` whose transmission type is `MSG` completes the pending send,
  cancels its timeout, and schedules restoration after 800 ms.
- A 15-second timeout schedules restoration when no matching acknowledgement
  arrives.
- While a send is pending, another queued 0x4504 request remains queued until
  the first send has restored its resources.

## Output State

The PWR layer owns output suspension. It records the current physical LED
state and whether flashing was active, closes audio, and prevents the periodic
flash task from lighting LEDs during transmission. On restoration it reopens
audio and either resumes the current flash mode, leaves a stopped flash off,
or restores the previous static LED state. Soft power-off always wins and
prevents restoration from turning outputs back on.

## Diagnostics

Selected RDSS test logs report the wait, acknowledgement restore scheduling,
timeout, and completed restoration. Existing 4505 acknowledgement notification
behavior remains unchanged.

## Scope

The change is limited to `HAL/decode.c`, `HAL/include/decode.h`, `HAL/PWR.c`,
and `HAL/include/PWR.h`. It does not change the App, 0x4504 layout, message
payload, frequency fallback, DM229 acknowledgement contents, or BLE notify
format.

## Verification

A source regression check verifies event ownership and ordering, the MSG-only
acknowledgement gate, timeout fallback, absence of immediate RNSS restoration,
and PWR output suspend/restore behavior. Existing RDSS, GNSS, BLE-disconnect,
and relevant voice checks remain part of regression verification.
