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
    decode_c = read_text("HAL/decode.c")
    start = decode_c.index("uint16_t RDSS_ProcessEvent")
    end = decode_c.index("void RNSS_init", start)
    rdss_process = decode_c[start:end]

    init_index = rdss_process.index("uint8_t rdss_lf = Msg_tx.lf;")
    assert "(rdss_lf != 2)" in rdss_process
    assert "(rdss_lf != 3)" in rdss_process
    assert "(rdss_lf != 5)" in rdss_process
    fallback_index = rdss_process.index("rdss_lf = 3;", init_index)
    command_index = rdss_process.index(
        'sprintf(strCCMSG,"$CCMSG,%s,%d,%d,", ICcard,rdss_lf,Msg_tx.encode)',
        fallback_index,
    )
    assert init_index < fallback_index < command_index
    assert re.search(r"\bMsg_tx\.lf\s*=(?!=)", rdss_process) is None
    assert "app_lf=%d tx_lf=%d" in rdss_process
    assert "ICcard, Msg_tx.lf, rdss_lf, Msg_tx.encode" in rdss_process

    print("PASS: RDSS invalid lf values fall back to DM229 LF2 value 3")


if __name__ == "__main__":
    main()
