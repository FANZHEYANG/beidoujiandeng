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


def function_definition(source, signature):
    start = source.index(signature)
    brace_start = source.index("{", start)
    depth = 0
    for index in range(brace_start, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start : index + 1]
    raise AssertionError(f"unterminated function: {signature}")


def should_trigger(snr_values, countdown, already_triggered=False, tx_pending=False):
    strongest = sorted(snr_values, reverse=True)
    return (
        not already_triggered
        and not tx_pending
        and countdown == 0
        and len(strongest) >= 3
        and strongest[2] >= 48
    )


def main():
    config_h = read_text("HAL/include/CONFIG.h")
    decode_c = read_text("HAL/decode.c")

    assert "#ifndef RDSS_SNR_AUTO_SEND_TEST" in config_h
    assert "#define RDSS_SNR_AUTO_SEND_TEST             TRUE" in config_h

    helper = function_definition(
        decode_c, "static void Rdss_TryQueueSnrAutoSendTest(void)"
    )
    assert "#if RDSS_SNR_AUTO_SEND_TEST" in helper
    assert "rdss_snr_auto_send_triggered" in helper
    assert "rdss_frequency_countdown_valid == 0" in helper
    assert "snr[0] < 48" in helper
    assert "snr[1] < 48" in helper
    assert "snr[2] < 48" in helper
    assert "frequency_count_down != 0" in helper
    assert "RD_txflag != 0" in helper
    assert "rdss_send_state != RDSS_SEND_IDLE" in helper
    assert "Pwr_IsSoftPowerOff() != FALSE" in helper

    assert 'strcpy((char *)DestIC, "4209606");' in helper
    assert "Msg_tx.lf = 3;" in helper
    assert "Msg_tx.encode = 2;" in helper
    assert "Msg_tx.generation = 3;" in helper
    assert "Msg_tx.heart_rate = 0;" in helper
    assert "Msg_tx.blood_oxygen = 0;" in helper
    assert "Msg_tx.foot_step = 0;" in helper
    assert "Msg_tx.kcal = 0;" in helper
    assert "Msg_tx.dest_card = 4209606UL;" in helper
    assert "Msg_tx.payload_len = 4;" in helper
    for index, value in enumerate(("0xB2", "0xE2", "0xCA", "0xD4")):
        assert f"Msg_tx.payload[{index}] = {value};" in helper

    trigger_pos = helper.index("rdss_snr_auto_send_triggered = 1;")
    flag_pos = helper.index("RD_txflag = TRUE;")
    event_pos = helper.index("tmos_set_event(RDSS_TaskID, rdss_evt);")
    assert trigger_pos < flag_pos < event_pos
    assert "Pwr_EnableRdssForSend();" in helper
    assert "[RDSS SNR TEST] trigger" in helper
    assert "[RDSS SNR TEST] queued dest=4209606 payload=B2 E2 CA D4" in helper

    bdsnr_start = decode_c.index('else if(strstr((char *)RD_PARSE_BUF,"$BDSNR"))')
    bdsnr_end = decode_c.index("tmos_start_task(RDSS_TaskID,rdss_evt", bdsnr_start)
    bdsnr_block = decode_c[bdsnr_start:bdsnr_end]
    assert bdsnr_block.index("snr[i] = strstr_show[i];") < bdsnr_block.index(
        "Rdss_TryQueueSnrAutoSendTest();"
    )
    bdttc_start = decode_c.index('else if(strstr((char *)RD_PARSE_BUF,"$BDTTC"))')
    bdttc_end = decode_c.index("//信噪比数组", bdttc_start)
    bdttc_block = decode_c[bdttc_start:bdttc_end]
    assert "if(RD_result != NULL)" in bdttc_block
    assert "rdss_frequency_countdown_valid = 1;" in bdttc_block

    assert not should_trigger([52, 48, 47, 0], countdown=0)
    assert should_trigger([48, 52, 49, 0], countdown=0)
    assert not should_trigger([52, 50, 49, 0], countdown=1)
    assert not should_trigger([52, 50, 49, 0], countdown=0, already_triggered=True)
    assert not should_trigger([52, 50, 49, 0], countdown=0, tx_pending=True)

    print("PASS: RDSS SNR test queues one fixed GBK message through the normal send path")


if __name__ == "__main__":
    main()
