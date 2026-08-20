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


def function_definition(source, signature):
    start = source.index(f"{signature}\n{{")
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


def main():
    config_h = read_text("HAL/include/CONFIG.h")
    hal_h = read_text("HAL/include/HAL.h")
    decode_c = read_text("HAL/decode.c")
    peripheral_c = read_text("Peripheral/APP/peripheral.c")
    profile_c = read_text("Peripheral/Profile/gattprofile.c")
    pwr_c = read_text("HAL/PWR.c")

    assert "#define SOS_TEST_ONLY_PRINT                 FALSE" in config_h
    assert "#define RDSS_SEND_TEST_ONLY_PRINT          TRUE" in config_h
    assert "#if RDSS_SEND_TEST_ONLY_PRINT" in hal_h
    assert "#define RDSS_SEND_TEST_PRINT(...) (printf)(__VA_ARGS__)" in hal_h
    assert "#undef PRINT" in hal_h
    assert re.search(r"(?m)^#define PRINT\(\.\.\.\)\s*$", hal_h)
    assert re.search(r"(?m)^#define printf\(\.\.\.\)\s*$", hal_h)

    write_start = profile_c.index("case RDSSPROFILE_CHAR4_UUID")
    write_end = profile_c.index("break;", write_start)
    write_4504 = profile_c[write_start:write_end]
    assert "[RDSS 4504 WRITE FAIL]" in write_4504
    assert "[RDSS 4504 WRITE OK]" in write_4504
    assert "RDSS_SEND_TEST_PRINT" in write_4504
    assert "PROFILE_DEBUG_PRINTF" not in write_4504

    rdss_start = decode_c.index("uint16_t RDSS_ProcessEvent")
    rdss_end = decode_c.index("void RNSS_init", rdss_start)
    rdss_process = decode_c[rdss_start:rdss_end]
    for label in (
        "[RDSS TX 4504]",
        "[RDSS TX WAIT]",
        "[RDSS TX TIMEOUT]",
        "[RDSS ACK BDFKI]",
        "[RDSS RX BDMXX]",
    ):
        assert label in rdss_process
    assert "[RDSS TX RESTORE]" in decode_c
    assert "[RDSS TX GUARD]" in decode_c
    assert rdss_process.count("RDSS_SEND_TEST_PRINT") == 11
    assert decode_c.count("RDSS_SEND_TEST_PRINT") == 16
    assert not re.search(
        r'(?m)^\s*PRINT\s*\("(?:\\r\\n)?\[RDSS (?:TX 4504|TX WAIT|TX TIMEOUT|TX RESTORE|TX GUARD|ACK BDFKI|RX BDMXX)\]',
        decode_c,
    )

    notify_4505 = function_definition(
        peripheral_c,
        "static uint8_t peripheralRdssTxAckNotify(uint8_t *pValue, uint16_t len)",
    )
    notify_4506 = function_definition(
        peripheral_c,
        "static uint8_t peripheralRdssMsgRxNotify(uint8_t *pValue, uint16_t len)",
    )
    for label in (
        "[4505] notify too large",
        "[4505] notify fail",
        "[4505] notify ok",
        "[4505] notify alloc fail",
    ):
        assert label in notify_4505
    for label in (
        "[4506] notify too large",
        "[4506] notify fail",
        "[4506] notify ok",
        "[4506] notify alloc fail",
    ):
        assert label in notify_4506
    direct_print = re.compile(r"(?m)^\s*PRINT\s*\(")
    assert direct_print.search(notify_4505) is None
    assert direct_print.search(notify_4506) is None
    assert notify_4505.count("RDSS_SEND_TEST_PRINT") == 4
    assert notify_4506.count("RDSS_SEND_TEST_PRINT") == 4
    assert peripheral_c.count("RDSS_SEND_TEST_PRINT") == 8
    assert profile_c.count("RDSS_SEND_TEST_PRINT") == 16

    assert '#define GNSS_DEBUG_PRINT_ENABLE 0' in decode_c
    assert 'PRINT("Connected..\\n")' in peripheral_c
    assert 'PRINT("[WATER] raw=%u mv=%u' in pwr_c

    for path in (
        "HAL/decode.c",
        "HAL/MCU.c",
        "HAL/PWR.c",
        "HAL/RTC.c",
        "Peripheral/APP/peripheral.c",
        "Peripheral/APP/peripheral_main.c",
        "Peripheral/Profile/gattprofile.c",
    ):
        assert '#include "HAL.h"' in read_text(path)


if __name__ == "__main__":
    main()
