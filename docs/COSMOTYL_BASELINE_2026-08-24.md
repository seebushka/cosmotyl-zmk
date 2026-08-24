# Cosmotyl Known-Good Baseline — 2026-08-24

## Baseline identity

- Git tag: `cosmotyl-baseline-2026-08-24`
- Known-good firmware commit: `0dd188d`
- ZMK: `v0.3.0`
- Zephyr: `v3.5.0+zmk-fixes`
- Repository: `seebushka/cosmotyl-zmk`

The Git tag points to the exact tested firmware state.
This document is the human-readable record of that baseline.

## Architecture

Three-device wireless ZMK system:

- LEFT: nice!nano V2, BLE peripheral
- RIGHT: nice!nano V2, BLE peripheral
- Receiver: Seeed XIAO nRF52840, BLE central + USB HID

Important:

- nice!nano builds MUST use `board: nice_nano_v2`
- `board: nice_nano` caused false 0% battery reporting
- PC communication is through the permanent USB XIAO receiver

## Power

Each keyboard half:

- 2000 mAh LiPo
- hardware power switch
- nice!nano V2 onboard charging

Battery reporting was validated against multimeter measurements.

Validation example:

- LEFT: 3.92 V, ZMK reported 92%
- RIGHT: 3.90 V, ZMK reported 100%

## LEFT matrix

5 rows x 7 columns.

Rows:

- R0 P0.31
- R1 P0.29
- R2 P0.02
- R3 P1.15
- R4 P1.13

Columns:

- C0 P0.17
- C1 P0.20
- C2 P0.22
- C3 P0.24
- C4 P1.00
- C5 P0.11
- C6 P1.04

Diode direction: `col2row`

## LEFT encoder

Bourns PEC11R rotary encoder.

- A: P0.10
- B: P1.11
- common: GND
- `steps = <96>;`
- `triggers-per-rotation = <24>;`

Final behavior:

- clockwise = volume up
- counterclockwise = volume down

Firmware emits:

- Ctrl+Alt+Shift+PageUp
- Ctrl+Alt+Shift+PageDown

GNOME maps these to volume up/down.

## RIGHT matrix

5 rows x 8 columns.

Rows:

- R0 P0.17
- R1 P0.20
- R2 P0.22
- R3 P0.24
- R4 P1.00

Columns:

- C0 P0.31
- C1 P0.29
- C2 P0.02
- C3 P1.15
- C4 P1.13
- C5 P1.11
- C6 P0.10
- C7 P0.09

Diode direction: `col2row`

No encoder on RIGHT.

## Custom physical function keys

- SOUND = F21
- LIGHT = F19
- REPLAY = Alt+F10
- A1 MINI = F20
- ARM = Ctrl+Shift+Alt+Q
- REC = Alt+F9

Fedora integrations:

- F21 -> `~/.local/bin/toggle-audio.sh`
- F20 -> `~/.local/bin/toggle-printer`
- F19 -> Hue/light binding

GPU Screen Recorder:

- REPLAY save = Alt+F10
- REC start/stop = Alt+F9
- ARM replay buffer = Ctrl+Shift+Alt+Q

All six physical function keys passed live testing.

## Receiver status LEDs

Two addressable APA106/WS2812-style LEDs.

Physical mapping:

- pixel 1 = LEFT indicator
- pixel 0 = RIGHT indicator

ZMK source mapping:

- source 0 = LEFT
- source 1 = RIGHT

Confirmed color mapping:

- RED
- GREEN
- BLUE

Brightness baseline: approximately 6/255.

Status behavior:

- solid green = connected, battery healthy
- solid amber = connected, battery low
- solid red = connected, battery critical
- slow blue blink = disconnected/searching
- off = receiver has no power

Thresholds:

- red = 0-15%
- amber = 16-30%
- green = above 30%

Disconnect blink:

- 750 ms on
- 750 ms off

Validation passed:

- LEFT off -> LEFT LED blinks blue
- RIGHT off -> RIGHT LED blinks blue
- reconnect -> correct LED returns to battery-status colour
- both healthy -> both LEDs solid green

## Receiver LED hardware

XIAO nRF52840:

- LED data = D0 / P0.02
- SPI SCK parked on D1 / P0.03
- physical LED chain uses the data connection

Driver:

- `worldsemi,ws2812-spi`
- chain length = 2
- SPI frequency = 4 MHz

## Final firmware artifacts

Production artifacts:

- `cosmotyl-left-wireless.uf2`
- `cosmotyl-right-wireless.uf2`
- `cosmotyl-dongle.uf2`

Recovery/test artifacts also exist for:

- nice!nano settings reset
- XIAO settings reset
- controller/matrix testing
- encoder pin testing

## Tested and accepted

Known-good at this baseline:

- LEFT matrix
- RIGHT matrix
- both thumb clusters
- six custom function keys
- red Ctrl+Shift key
- encoder
- LEFT BLE -> dongle -> USB
- RIGHT BLE -> dongle -> USB
- LEFT encoder -> RF -> dongle -> USB
- reconnect behavior
- battery reporting
- receiver LED physical mapping
- receiver LED RGB mapping
- receiver connection indication
- receiver battery indication
- USB HID output

## Recovery point

Inspect the exact known-good source with:

    git checkout cosmotyl-baseline-2026-08-24

Create a new development branch from it with:

    git switch -c <new-branch-name> cosmotyl-baseline-2026-08-24

Do not move or recreate the baseline tag.

## Development rule

`main` may continue evolving.

If a future change causes a regression:

1. compare against `cosmotyl-baseline-2026-08-24`
2. identify the changed subsystem
3. restore or cherry-pick from the known-good state as required

The baseline tag is the firmware safety net.
