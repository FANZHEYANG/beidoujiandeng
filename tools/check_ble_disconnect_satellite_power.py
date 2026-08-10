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
    peripheral_c = read_text("Peripheral/APP/peripheral.c")

    link_terminated = function_definition(
        peripheral_c,
        "static void Peripheral_LinkTerminated(gapRoleEvent_t *pEvent)",
    )
    link_established = function_definition(
        peripheral_c,
        "static void Peripheral_LinkEstablished(gapRoleEvent_t *pEvent)",
    )
    ble_off = function_definition(peripheral_c, "void Peripheral_BleOff(void)")
    ble_on = function_definition(peripheral_c, "void Peripheral_BleOn(void)")
    satellite_helper = function_definition(
        peripheral_c,
        "static void peripheralSetSatelliteModules(BOOL enabled)",
    )

    assert "peripheralSetSatelliteModules(TRUE);" in link_established
    assert "peripheralSetSatelliteModules(FALSE);" not in link_terminated
    for forbidden in (
        "CLOSERN();",
        "CLOSERD();",
        "RN_SW_Flag = FALSE;",
        "RD_SW_Flag = FALSE;",
    ):
        assert forbidden not in link_terminated
    assert "peripheralSetSatelliteModules(FALSE);" in ble_off
    assert "peripheralSetSatelliteModules(FALSE);" in ble_on
    assert "RN_SW_Flag = enabled;" in satellite_helper
    assert "RD_SW_Flag = enabled;" in satellite_helper
    assert "OPENRN();" in satellite_helper
    assert "OPENRD();" in satellite_helper
    assert "CLOSERN();" in satellite_helper
    assert "CLOSERD();" in satellite_helper


if __name__ == "__main__":
    main()
