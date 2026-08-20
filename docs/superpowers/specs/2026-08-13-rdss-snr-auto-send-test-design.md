# RDSS SNR Auto-Send Test Design

## Goal

Add an isolated firmware test that automatically sends the GBK text `测试` once through DM229 to the local card `4209606` after usable RDSS signal is detected. The test must bypass APP writes while still exercising the board's real DM229 send and acknowledgement path.

## Trigger

- Add compile-time switch `RDSS_SNR_AUTO_SEND_TEST` in `HAL/include/CONFIG.h`.
- Enable it for this test build. Setting it to `FALSE` must remove the automatic behavior without changing the normal 0x4504 path.
- Evaluate the trigger after a complete `$BDSNR` frame has been parsed and its 21 values have been sorted in descending order.
- Trigger only after a valid `$BDTTC` has been parsed, the three strongest channels are all at least 48, and `frequency_count_down` is 0. The initial zero value is not treated as a valid countdown.
- Trigger at most once per power-on. A failed acknowledgement or timeout does not automatically retry.

## Test Message

- Destination: `4209606`.
- `lf`: 3.
- `encode`: 2.
- `generation`: 3.
- Payload: `B2 E2 CA D4`, the GBK encoding of `测试`.
- Heart rate, blood oxygen, steps, and calories remain zero.

## Data Flow

1. DM229 emits `$BDSNR` and the existing parser updates the sorted `snr[]` values.
2. The test helper checks the threshold, frequency countdown, one-shot flag, soft-power state, and whether another RDSS message is already queued or being sent.
3. When all checks pass, it fills `DestIC` and `Msg_tx`, powers the RDSS channel, sets `RD_txflag`, marks the test as triggered, and schedules the existing RDSS task.
4. The existing send state machine closes RNSS, suspends audio and LEDs, generates `$CCMSG`, sends through DM229, waits for a valid `$BDFKI,MSG`, reports 0x4505 when APP notifications are available, and restores RNSS and outputs.

## Isolation And Safety

- Do not write UART directly from the SNR parser.
- Do not overwrite an APP, SOS, or location message that is queued or in progress.
- Do not trigger while soft-powered off.
- Normal APP 0x4504 sending remains unchanged.
- The test is deliberately one-shot so recurring `$BDSNR` reports cannot repeatedly transmit.

## Diagnostics

Add RDSS test-only logs for:

- The three strongest SNR values and current frequency countdown when the trigger fires.
- The fixed destination and payload bytes queued by the test.
- Existing `$CCMSG`, `$BDFKI`, timeout, cooldown, and restore logs remain the source of the final result.

Expected successful sequence includes:

```text
[RDSS SNR TEST] trigger snr=...,...,... countdown=0
[RDSS SNR TEST] queued dest=4209606 payload=B2 E2 CA D4
[RDSS TX 4504] ...
[RDSS TX WAIT] ...
[RDSS ACK BDFKI] ack=1 reason=0
[RDSS TX RESTORE] complete rn=1
```

## Verification

- Add a source regression check before implementation and confirm it fails because the test feature is absent.
- Verify the check passes after implementation.
- Run existing RDSS send, acknowledgement restore, GNSS print, BLE power, voice queue, and Python syntax checks.
- Run `git diff --check`.
- Target firmware compilation remains dependent on the external MounRiver/WCH RISC-V toolchain, which is not currently available in the command environment.
