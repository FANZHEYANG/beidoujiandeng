# Keep Satellite Modules On BLE Disconnect Design

## Goal

Keep RNSS and RDSS powered after an ordinary Bluetooth link disconnect so
satellite and water-voltage testing can continue without reconnecting the app.

## Behavior

- A successful Bluetooth connection continues to enable RNSS and RDSS.
- An ordinary Bluetooth link termination clears connection state and restarts
  advertising, but does not change RNSS/RDSS power flags or GPIO outputs.
- Explicit Bluetooth shutdown, system shutdown, and cold-start behavior remain
  unchanged and may still turn both satellite modules off.
- RDSS transmit-time RNSS switching remains unchanged.

## Verification

- A focused source check verifies the link-termination handler no longer calls
  `peripheralSetSatelliteModules(FALSE)`.
- The same check verifies link establishment still enables both modules and
  explicit Bluetooth shutdown still disables both modules.

