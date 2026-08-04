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
    decode_c = read_text("HAL/decode.c")
    mcu_c = read_text("HAL/MCU.c")
    peripheral_h = read_text("Peripheral/APP/include/peripheral.h")
    peripheral_c = read_text("Peripheral/APP/peripheral.c")
    gattprofile_c = read_text("Peripheral/Profile/gattprofile.c")
    gattprofile_h = read_text("Peripheral/Profile/include/gattprofile.h")
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
    periodic_task_start = peripheral_c.rindex("static void performPeriodicTask")
    periodic_task = peripheral_c[
        periodic_task_start:
        peripheral_c.index("/*********************************************************************", periodic_task_start)
    ]
    parse_gga = decode_c[
        decode_c.index("void parseGpsBuffer"):
        decode_c.index("void printGpsBuffer")
    ]
    print_gga = decode_c[
        decode_c.index("void printGpsBuffer"):
        decode_c.index("uint16_t RNSS_ProcessEvent")
    ]
    rdss_process = decode_c[
        decode_c.index("uint16_t RDSS_ProcessEvent"):
        decode_c.index("void RDSS_init")
    ]
    rdss_write_4504 = gattprofile_c[
        gattprofile_c.index("case RDSSPROFILE_CHAR4_UUID"):
        gattprofile_c.index("case GATT_CLIENT_CHAR_CFG_UUID", gattprofile_c.index("case RDSSPROFILE_CHAR4_UUID"))
    ]
    rdss_read_4506 = gattprofile_c[
        gattprofile_c.index("case RDSSPROFILE_CHAR6_UUID"):
        gattprofile_c.index("case RDSSPROFILE_CHAR7_UUID", gattprofile_c.index("case RDSSPROFILE_CHAR6_UUID"))
    ]
    rdss_notify_all = peripheral_c[
        peripheral_c.index("static void sendRdssNotifications"):
        peripheral_c.index("void Peripheral_BleOff")
    ]
    sos_queue_start = pwr_c.index("static uint8_t Pwr_QueueSosMessage")
    sos_queue = pwr_c[
        sos_queue_start:
        pwr_c.index("static void Pwr_StartSosAlarm", sos_queue_start)
    ]
    clear_gps_fields = decode_c[
        decode_c.index("static void clearGpsParsedFields"):
        decode_c.index("static void clearGgaPositionFields")
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
    assert "#define GNSSPROFILE_CHAR1_LEN     12" in gattprofile_h, "0x2A6A must keep the 12-byte RNSS SNR format"
    assert "#define GNSSPROFILE_CHAR2_LEN     40" in gattprofile_h, "0x2A67 must keep the 40-byte RNSS GGA format"
    assert "Save_GSV_Data.satellites[i].snr" in periodic_task, "0x2A6A notify must use parsed GSV SNR values"
    assert "peripheralGnssChar1Notify(gnssData1, GNSSPROFILE_CHAR1_LEN);" in periodic_task, "RNSS SNR must be notified on 0x2A6A"
    assert "GGA.latitude" in periodic_task and "GGA.longitude" in periodic_task, "0x2A67 must carry parsed decimal-degree coordinates"
    assert "peripheralGnssChar2Notify(gnssData2, GNSSPROFILE_CHAR2_LEN);" in periodic_task, "RNSS GGA must be notified on 0x2A67"
    assert "static void clearGpsParsedFields" in decode_c, "each new GGA frame must clear old parsed text fields"
    assert "memset(Save_Data.UTCTime, 0, UTCTime_Length);" in clear_gps_fields, "GGA text-field clear helper must clear UTC time"
    assert clear_gps_fields.count("clearGpsParsedFields();") == 0, "GGA text-field clear helper must not recurse"
    assert "static void clearGgaPositionFields" in decode_c, "invalid GGA fixes must clear old output coordinates"
    assert "static void gnssCopyField" in decode_c, "GGA parser must copy fields with bounds and null termination"
    assert "clearGpsParsedFields();" in parse_gga, "GGA parser must clear old text fields before parsing a new frame"
    assert "Save_Data.isParseData = false;" in parse_gga, "GGA parser must not reuse a stale parsed flag"
    assert "GGA.fix_quality = 0;" in parse_gga, "GGA parser must reset fix quality before parsing a new frame"
    assert "GGA.fix_quality = atoi((char *)usefullBuffer);" in parse_gga, "GGA parser must preserve the module fix-quality value"
    assert "Save_Data.isUsefull = (GGA.fix_quality > 0) ? true : false;" in parse_gga, "any non-zero GGA fix quality must be treated as located"
    assert "gnssCopyField(Save_Data.latitude, latitude_Length, subString, field_end);" in parse_gga, "latitude field must be copied safely"
    assert "gnssCopyField(Save_Data.longitude, longitude_Length, subString, field_end);" in parse_gga, "longitude field must be copied safely"
    assert "clearGgaPositionFields();" in print_gga, "unfixed GGA output must not keep old coordinates"
    assert "#define RDSS_MSG_PAYLOAD_MAX 70" in pwr_h or "#define RDSS_MSG_PAYLOAD_MAX 70" in read_text("HAL/include/decode.h"), "RDSS SMS payload limit must be named"
    assert "RDSS_ACK_REASON_PAYLOAD_TOO_LONG" in read_text("HAL/include/decode.h"), "oversized SMS writes must have a local ACK reason"
    assert "uint16_t Rdss_SanitizePayloadLen" in read_text("HAL/include/decode.h"), "RDSS payload length sanitizer must be public"
    assert "uint16_t Rdss_SanitizePayloadLen" in decode_c, "RDSS payload length sanitizer must be implemented"
    assert "RD_msg_rx_dirty" in read_text("HAL/include/decode.h"), "new inbound SMS state must be exposed"
    assert "uint8_t  RD_msg_rx_dirty = 0;" in decode_c, "new inbound SMS state must be stored"
    assert "payload[RDSS_MSG_PAYLOAD_MAX]" in read_text("HAL/include/decode.h"), "outbound SMS buffer must use the named limit"
    assert "Msg_tx.payload_len = Rdss_SanitizePayloadLen" in rdss_write_4504, "4504 writes must sanitize GB2312 payload boundaries"
    assert "requested_payload_len" in rdss_write_4504, "4504 writes must compare requested and accepted payload lengths"
    assert "Tx_ack.reason = RDSS_ACK_REASON_PAYLOAD_TOO_LONG;" in rdss_write_4504, "oversized 4504 writes must report failure to APP"
    assert "RD_tx_ack_dirty = 1;" in rdss_write_4504, "local 4504 failures must trigger 4505 notify"
    assert "RD_txflag = true;" in rdss_write_4504, "valid 4504 writes must still queue DM229 transmission"
    assert "Msg_tx.payload_len = Rdss_SanitizePayloadLen" in rdss_process, "DM229 transmit path must keep payload on a safe boundary"
    assert "Rdss_ClearMsgRx();" in rdss_process, "BDMXX parsing must clear stale inbound SMS fields"
    assert "RD_msg_rx_dirty = 1;" in rdss_process, "new BDMXX messages must mark 4506 dirty"
    assert "j<RDSS_MSG_PAYLOAD_MAX" in rdss_process, "BDMXX payload copied to APP must stay within 4506 payload capacity"
    assert "if(RD_msg_rx_dirty)" in rdss_notify_all, "4506 must notify only after a new inbound SMS"
    assert "RD_msg_rx_dirty = 0;" in rdss_notify_all, "4506 dirty flag must clear after notify succeeds"
    assert "memset(read_data6, 0, RDSSPROFILE_CHAR6_LEN);" in rdss_read_4506, "4506 reads must not leak stale payload bytes"
    assert "payload_copy_len = Rdss_SanitizePayloadLen" in rdss_read_4506, "4506 reads must publish a safe payload length"
    assert "SOS,LAT=%s,LON=%s,HR=0,O2=0,STEP=0,KCAL=0" in sos_queue, "SOS SMS must include position and vital placeholders without Chinese text"
    assert "payload[RDSS_MSG_PAYLOAD_MAX + 1]" in sos_queue, "SOS SMS buffer must fit the full RDSS payload plus terminator"
    assert "Pwr_StopSosAlarm(SOS_EXIT_SEND_MAX);" in sos_queue, "SOS must still exit after the configured send limit"


if __name__ == "__main__":
    main()
