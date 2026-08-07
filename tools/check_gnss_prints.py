import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def read_text(path):
    data = (ROOT / path).read_bytes()
    for encoding in ("utf-8", "gbk", "latin1"):
        try:
            return data.decode(encoding)
        except UnicodeDecodeError:
            continue
    return data.decode("latin1")


def main():
    decode_c = read_text("HAL/decode.c")
    gnss_start = decode_c.index("int parse_gbgsv")
    gnss_end = decode_c.index("uint16_t RDSS_ProcessEvent", gnss_start)
    gnss_section = decode_c[gnss_start:gnss_end]

    assert "#define GNSS_DEBUG_PRINT_ENABLE 0" in decode_c
    assert "#define GNSS_DEBUG_PRINT(...) PRINT(__VA_ARGS__)" in decode_c
    assert re.search(r"(?m)^#define GNSS_DEBUG_PRINT\(\.\.\.\)\s*$", decode_c)

    mute_start = decode_c.index("static void gnss_mute_rx_ms")
    mute_end = decode_c.index("static uint8_t gnss_ring_has_echo", mute_start)
    mute_section = decode_c[mute_start:mute_end]
    assert "#if GNSS_DEBUG_PRINT_ENABLE" in mute_section
    assert "(void)ms;" in mute_section
    assert "return 0;" in mute_section

    assert 'GNSS_DEBUG_PRINT("\\r\\n[GNSS GSV] count=%d"' in decode_c
    assert 'GNSS_DEBUG_PRINT("\\r\\n[GNSS GGA] fix=%d lat=%f lon=%f sats=%d\\r\\n"' in decode_c
    assert 'GNSS_DEBUG_PRINT("[GNSS] rx=%lu had_gga=%d gsv=%d/%d sat=%d fix=%d buf=%u\\r\\n"' in decode_c
    assert 'GNSS_DEBUG_PRINT("[GNSS RAW] ")' in decode_c

    direct_print = re.compile(r"(?m)^\s*(?:PRINT|printf)\s*\(")
    assert direct_print.search(gnss_section) is None
    assert 'printf("GSV checksum warn' not in decode_c
    assert 'printf("ERROR\\r\\n")' not in decode_c

    assert 'PRINT("\\r\\n[RDSS TX 4504]' in decode_c
    assert 'PRINT("\\r\\n[RDSS ACK BDFKI]' in decode_c


if __name__ == "__main__":
    main()
