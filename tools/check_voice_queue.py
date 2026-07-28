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
    mcu_c = read_text("HAL/MCU.c")
    peripheral_c = read_text("Peripheral/APP/peripheral.c")
    soft_power_on = pwr_c[pwr_c.index("static void SoftPowerOn"):pwr_c.index("//PWR ADC")]
    pwr_init = pwr_c[pwr_c.index("void Pwr_init"):pwr_c.index("//BAT ADC")]
    queue_voice = pwr_c[pwr_c.index("static uint8_t Pwr_QueueVoiceText"):pwr_c.index("uint8_t Pwr_RequestVoiceText")]
    sos_event = pwr_c[pwr_c.index("if(events & sos_evt)"):pwr_c.index("if(events & key1_evt)")]

    assert "Pwr_RequestVoiceText" in pwr_h, "PWR.h must expose the voice queue API"
    assert "uint8_t Pwr_RequestVoiceText" in pwr_c, "PWR.c must implement the voice queue API"
    assert "return Pwr_QueueVoiceText(text);" in pwr_c, "public voice API must use the queue"
    assert 'Audio_play("' not in mcu_c, "MCU init must not bypass the voice queue"
    assert "Pwr_RequestVoiceText(" in pwr_init, "Pwr_init must queue the initial startup voice"
    assert "tmos_start_task(Pwr_TaskID,sos_evt,1600)" not in pwr_init, "Pwr_init must not delay the queued startup voice"
    assert "voice_playing" in pwr_c, "voice queue must track playback state"
    assert "(voice_playing == 0)" in queue_voice, "queueing while playing must not restart the voice timer"
    assert "voice_playing = 1;" in sos_event, "voice playback must mark the queue busy"
    assert "voice_playing = 0;" in sos_event, "voice event must release the busy state before checking the queue"
    assert 'Audio_play("' not in soft_power_on, "startup voice must not bypass the voice queue"
    assert "Pwr_RequestVoiceText(" in soft_power_on, "startup voice must be queued first"
    assert "Audio_play(" not in peripheral_c, "peripheral.c must not bypass the voice queue"
    assert peripheral_c.count("Pwr_RequestVoiceText(") >= 2, "BLE connect/disconnect voices must be queued"


if __name__ == "__main__":
    main()
