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
    pwr_h = read_text("HAL/include/PWR.h")
    pwr_c = read_text("HAL/PWR.c")
    peripheral_c = read_text("Peripheral/APP/peripheral.c")
    soft_power_on = pwr_c[pwr_c.index("static void SoftPowerOn"):pwr_c.index("//PWR ADC")]

    assert "Pwr_RequestVoiceText" in pwr_h, "PWR.h must expose the voice queue API"
    assert "uint8_t Pwr_RequestVoiceText" in pwr_c, "PWR.c must implement the voice queue API"
    assert "return Pwr_QueueVoiceText(text);" in pwr_c, "public voice API must use the queue"
    assert 'Audio_play("' not in soft_power_on, "startup voice must not bypass the voice queue"
    assert "Pwr_RequestVoiceText(" in soft_power_on, "startup voice must be queued first"
    assert "Audio_play(" not in peripheral_c, "peripheral.c must not bypass the voice queue"
    assert peripheral_c.count("Pwr_RequestVoiceText(") >= 2, "BLE connect/disconnect voices must be queued"


if __name__ == "__main__":
    main()
