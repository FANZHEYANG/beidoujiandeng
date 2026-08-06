# PA12 Water ADC Range Design

## Problem

PA12 measures 3.267 V with a multimeter, while firmware reports `raw=4084` and
`mv=4187`. The register diagnostics show the intended ADC2 channel and digital
input disable are active.

CH58x `ADC_PGA_1_2` uses the SDK conversion
`raw * 1050 / 1024 - 1050`, so its maximum input is about 3.15 V. A 3.267 V
input therefore saturates. The existing proportional conversion also omits the
ADC input offset.

## Design

- Sample PA12/ADC2 with `ADC_PGA_1_4`, whose SDK range covers up to about
  5.25 V.
- Convert the calibrated sample with
  `ADC_VoltConverSignalPGA_MINUS_12dB()` and clamp negative results to 0 mV.
- Calibrate the water channel after configuring its gain and store the result
  separately from the battery channel calibration.
- Keep the existing 2000 mV threshold, debounce counts, and LED flash behavior.
- Keep the diagnostic print and include the water calibration value.

At 3.267 V the expected reading is approximately `raw=3129`, `mv=3267`. At a
0.3 V short the expected converted voltage is approximately 300 mV.

## Verification

- A static regression check enforces the gain, independent calibration, and
  SDK conversion function.
- Run the existing firmware static checks.
- Attempt the project build and report if the local compiler tool is missing.
- Confirm on hardware that dry PA12 reads near the multimeter value and a short
  produces a value below 2000 mV and starts flashing after three samples.
