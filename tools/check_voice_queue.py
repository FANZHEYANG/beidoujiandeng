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
    peripheral_h = read_text("Peripheral/APP/include/peripheral.h")
    peripheral_c = read_text("Peripheral/APP/peripheral.c")
    soft_power_on = pwr_c[pwr_c.index("static void SoftPowerOn"):pwr_c.index("//PWR ADC")]
    pwr_init = pwr_c[pwr_c.index("void Pwr_init"):pwr_c.index("//BAT ADC")]
    queue_voice = pwr_c[pwr_c.index("static uint8_t Pwr_QueueVoiceText"):pwr_c.index("uint8_t Pwr_RequestVoiceText")]
    sos_event = pwr_c[pwr_c.index("if(events & sos_evt)"):pwr_c.index("if(events & key1_evt)")]
    pwr_process = pwr_c[pwr_c.index("uint16_t Pwr_ProcessEvent"):pwr_c.index("void Pwr_init")]
    peripheral_state_start = peripheral_c.rindex("static void peripheralStateNotificationCB")
    peripheral_state = peripheral_c[
        peripheral_state_start:
        peripheral_c.index("static void performPeriodicTask", peripheral_state_start)
    ]
    link_established = peripheral_c[
        peripheral_c.index("static void Peripheral_LinkEstablished"):
        peripheral_c.index("/*********************************************************************", peripheral_c.index("static void Peripheral_LinkTerminated"))
    ]
    link_terminated = peripheral_c[
        peripheral_c.index("static void Peripheral_LinkTerminated"):
        peripheral_state_start
    ]
    ble_off = peripheral_c[
        peripheral_c.index("void Peripheral_BleOff"):
        peripheral_c.index("void Peripheral_BleOn")
    ]
    ble_on = peripheral_c[
        peripheral_c.index("void Peripheral_BleOn"):
        peripheral_c.index("/*********************************************************************", peripheral_c.index("void Peripheral_BleOn"))
    ]

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
    assert "SBP_BLE_DISCONNECT_VOICE_EVT" in peripheral_h, "BLE disconnect voice must use a delayed event"
    assert "BLE_RECONNECT_SUPPRESS_PERIOD" in peripheral_c, "BLE reconnect suppression delay must be named"
    assert "MS1_TO_SYSTEM_TIME(2000)" in peripheral_c, "short BLE reconnects under 2 seconds must be suppressed"
    assert "ble_disconnect_voice_pending" in peripheral_c, "BLE disconnect voice must track a pending confirmation"
    assert "peripheralScheduleDisconnectVoice" in peripheral_c, "BLE disconnect voice must be scheduled, not played immediately"
    assert "peripheralHandleConnectedVoice" in peripheral_c, "BLE connected voice must be filtered through reconnect logic"
    assert "Pwr_RequestVoiceText(" not in peripheral_state, "BLE state callback must not queue voice before debounce"
    assert "peripheralScheduleDisconnectVoice();" in peripheral_state, "disconnect callback must schedule delayed voice"
    assert "peripheralHandleConnectedVoice();" in peripheral_state, "connect callback must use debounce helper"
    assert "Pwr_RequestAutoPowerOff" in pwr_h, "PWR.h must expose auto power-off request API"
    assert "void Pwr_RequestAutoPowerOff(void)" in pwr_c, "PWR.c must implement auto power-off request API"
    assert "auto_poweroff_evt" in pwr_h, "PWR must have an auto power-off task event"
    assert "auto_poweroff_evt" in pwr_process, "PWR task must handle auto power-off event"
    assert "SoftPowerOff();" in pwr_process, "auto power-off must use the existing soft power-off path"
    assert "SBP_BLE_AUTO_POWEROFF_EVT" in peripheral_h, "BLE app must have a 15-minute auto power-off event"
    assert "BLE_AUTO_POWEROFF_PERIOD" in peripheral_c, "BLE auto power-off delay must be named"
    assert "#define BLE_AUTO_POWEROFF_PERIOD" in peripheral_c, "BLE auto power-off delay must stay configurable"
    assert "peripheralScheduleAutoPowerOff" in peripheral_c, "BLE disconnect must schedule auto power-off"
    assert "peripheralCancelAutoPowerOff" in peripheral_c, "BLE reconnect must cancel auto power-off"
    assert "Pwr_RequestAutoPowerOff();" in peripheral_c, "BLE auto power-off event must request PWR soft shutdown"
    assert "BOOL RN_SW_Flag   = FALSE;" in pwr_c, "RNSS must stay off until BLE is connected"
    assert "BOOL RD_SW_Flag   = FALSE;" in pwr_c, "RDSS must stay off until BLE is connected"
    assert "peripheralSetSatelliteModules" in peripheral_c, "BLE layer must own satellite module power state"
    assert "RN_SW_Flag = enabled;" in peripheral_c, "satellite helper must update RNSS power flag"
    assert "RD_SW_Flag = enabled;" in peripheral_c, "satellite helper must update RDSS power flag"
    assert "OPENRN();" in peripheral_c and "OPENRD();" in peripheral_c, "satellite helper must power on RNSS/RDSS"
    assert "CLOSERN();" in peripheral_c and "CLOSERD();" in peripheral_c, "satellite helper must power off RNSS/RDSS"
    assert "peripheralSetSatelliteModules(TRUE);" in link_established, "BLE connect must enable RNSS/RDSS"
    assert "peripheralSetSatelliteModules(FALSE);" in link_terminated, "BLE disconnect must disable RNSS/RDSS"
    assert "peripheralSetSatelliteModules(FALSE);" in ble_off, "BLE off must disable RNSS/RDSS"
    assert "peripheralSetSatelliteModules(FALSE);" in ble_on, "BLE on must start advertising with RNSS/RDSS off"
    assert "peripheralRdssSnrNotify(snr, RDSSPROFILE_CHAR3_LEN);" in peripheral_c, "0x4503 notify must keep using parsed RDSS SNR"


if __name__ == "__main__":
    main()
