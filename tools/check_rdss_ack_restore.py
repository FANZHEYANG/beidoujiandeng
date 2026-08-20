import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def read_text(path):
    data = (ROOT / path).read_bytes()
    for encoding in ("utf-8", "gbk", "latin1"):
        try:
            return data.decode(encoding).replace("\r\n", "\n")
        except UnicodeDecodeError:
            continue
    return data.decode("latin1").replace("\r\n", "\n")


def main():
    decode_h = read_text("HAL/include/decode.h")
    decode_c = read_text("HAL/decode.c")
    pwr_h = read_text("HAL/include/PWR.h")
    pwr_c = read_text("HAL/PWR.c")

    assert "#define rdss_restore_evt" in decode_h
    assert "#define rdss_send_timeout_evt" in decode_h
    assert "#define RDSS_RN_RESTORE_DELAY MS1_TO_SYSTEM_TIME(800)" in decode_c
    assert "#define RDSS_SEND_ACK_TIMEOUT MS1_TO_SYSTEM_TIME(15000)" in decode_c
    assert "#define RDSS_LATE_ACK_GUARD MS1_TO_SYSTEM_TIME(5000)" in decode_c
    assert "RDSS_SEND_WAIT_ACK" in decode_c
    assert "RDSS_SEND_WAIT_RESTORE" in decode_c
    assert "RDSS_SEND_COOLDOWN" in decode_c
    assert "static uint8_t rdss_send_state = RDSS_SEND_IDLE;" in decode_c
    assert "static uint32_t RD_FRAME_SEQUENCE[RD_FRAME_QUEUE_SIZE];" in decode_c
    assert "static volatile uint32_t rd_frame_sequence = 0;" in decode_c
    assert "static uint32_t rdss_ack_min_sequence = 0;" in decode_c
    assert "RD_FRAME_SEQUENCE[RD_FRAME_WRITE] = rd_frame_sequence;" in decode_c

    process_start = decode_c.index("uint16_t RDSS_ProcessEvent")
    process_end = decode_c.index("void RNSS_init", process_start)
    rdss_process = decode_c[process_start:process_end]
    send_start = rdss_process.index("if ((RD_txflag==1)")
    send_end = rdss_process.index("while (RD_FRAME_COUNT > 0)", send_start)
    send_block = rdss_process[send_start:send_end]

    assert "rdss_send_state == RDSS_SEND_IDLE" in send_block
    assert "Pwr_IsSoftPowerOff() == FALSE" in send_block
    assert "Pwr_SuspendOutputsForRdssSend();" in send_block
    assert "rdss_send_state = RDSS_SEND_WAIT_ACK;" in send_block
    assert "rdss_ack_min_sequence = rd_frame_sequence + 1;" in send_block
    assert "RDSS_SEND_ACK_TIMEOUT" in send_block
    assert "OPENRN();" not in send_block
    assert "CLOSEAUDIO();" not in send_block
    assert "HalLedOnOff(HAL_LED_ALL, HAL_LED_MODE_OFF);" not in send_block

    ack_start = rdss_process.index('strstr((char *)RD_PARSE_BUF,"$BDFKI")')
    ack_tail = rdss_process[ack_start : ack_start + 4000]
    assert 'strcmp(RD_result,"MSG")' in ack_tail
    assert "Rdss_ScheduleRestore" in ack_tail
    assert "RDSS_RN_RESTORE_DELAY" in ack_tail
    assert "rdss_send_state == RDSS_SEND_WAIT_ACK" in ack_tail
    assert "rd_frame_seq >= rdss_ack_min_sequence" in ack_tail
    assert "if(ack_matches_pending)" in ack_tail
    assert ack_tail.index("if(ack_matches_pending)") < ack_tail.index("RD_tx_ack_dirty = 1;")
    assert "if(RD_result == NULL)" in ack_tail

    assert "static void Rdss_RestoreNow(void)" in decode_c
    assert "if(tmos_start_task(RDSS_TaskID, rdss_restore_evt, delay_ticks) == FALSE)" in decode_c
    assert "if(tmos_start_task(RDSS_TaskID, rdss_send_timeout_evt," in send_block
    assert "Rdss_RestoreNow();" in decode_c

    assert "if(events & rdss_send_timeout_evt)" in rdss_process
    assert 'Rdss_ScheduleRestore(1, "timeout", 1)' in rdss_process
    assert "if(events & rdss_restore_evt)" in rdss_process
    assert "if(rdss_send_state == RDSS_SEND_WAIT_RESTORE)" in rdss_process
    assert "else if(rdss_send_state == RDSS_SEND_COOLDOWN)" in rdss_process
    assert "RDSS_LATE_ACK_GUARD" in decode_c
    assert "RD_txflag = false;" in decode_c
    guard_fail_start = decode_c.index('[RDSS TX GUARD] timer unavailable')
    guard_fail_block = decode_c[guard_fail_start : guard_fail_start + 160]
    assert "RD_txflag = false;" in guard_fail_block
    assert "Rdss_QueueNextSend();" not in guard_fail_block
    assert "Pwr_RestoreOutputsAfterRdssSend();" in decode_c
    assert "Pwr_IsSoftPowerOff()" in decode_c
    assert "RN_SW_Flag = TRUE;" in decode_c
    assert "OPENRN();" in decode_c
    tx_event_start = rdss_process.index("if(events & rdss_tx_evt)")
    tx_event_block = rdss_process[tx_event_start:]
    assert "if(rdss_send_state == RDSS_SEND_IDLE)" in tx_event_block

    for prototype in (
        "extern void Pwr_SuspendOutputsForRdssSend(void);",
        "extern void Pwr_RestoreOutputsAfterRdssSend(void);",
        "extern BOOL Pwr_IsSoftPowerOff(void);",
    ):
        assert prototype in pwr_h

    assert "void Pwr_SuspendOutputsForRdssSend(void)" in pwr_c
    assert "void Pwr_RestoreOutputsAfterRdssSend(void)" in pwr_c
    assert "BOOL Pwr_IsSoftPowerOff(void)" in pwr_c
    soft_off_start = pwr_c.index("static void SoftPowerOff(void)")
    soft_off_end = pwr_c.index("void Pwr_RequestAutoPowerOff(void)", soft_off_start)
    soft_off_block = pwr_c[soft_off_start:soft_off_end]
    assert "RD_txflag = false;" in soft_off_block
    assert "rdss_output_suspended" in pwr_c
    assert "rdss_saved_led_state = HalLedGetState();" in pwr_c
    assert "rdss_led_flash_was_enabled = led_flash_enable;" in pwr_c
    assert re.search(
        r"if\(led_flash_enable && \(rdss_output_suspended == 0\)\)", pwr_c
    )
    assert (
        "if((rdss_output_suspended == 0) && Pwr_DequeueVoiceText(voice_text))"
        in pwr_c
    )

    print("PASS: RNSS and outputs restore only after BDFKI or timeout")


if __name__ == "__main__":
    main()
