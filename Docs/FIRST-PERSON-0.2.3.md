# Factory Scanner 0.2.3 — first-person presentation

The supplied 1920×1200 in-game capture shows the scanner centered around x=1125 and only about 380 pixels wide, while the visible hands span roughly 900 pixels. The adjustment recenters the equipment and doubles only its first-person scale. The intended result is that the two CAD handles approach the visible palms while the display remains readable.

- `FirstPersonOffset`: `(45, 0, -14)`; the former Y=13 offset placed the scanner too far right.
- `FirstPersonScale`: `2.0`; new and independent from world/third-person size.
- `EquipmentScale`: remains `1.0`, so inventory/world and remote-player presentation are not doubled.
- Both values remain configurable in `ItemScanner.ini` for the next screenshot pass.

The equipment already rotates its physical rotary dial by 30 degrees per wheel notch. A visible thumb turn is a separate first-person skeletal animation, not a static-mesh transform. The current stock portable-miner two-hand idle has no scanner-specific grip or thumb animation. A proper implementation needs a custom idle pose plus a short left/right thumb animation or montage made against the game's first-person hand skeleton. This revision does not fake that motion by moving the camera or whole arm.

Exact grip contact varies by FOV and must be checked in the game. The change is deliberately config-driven so one screenshot can be used for a quick final correction.

## Input changes

- Plain `1` through `5` remain available to Satisfactory's hotbar/build controls.
- `Shift+1` through `Shift+4` toggle the four scanner categories.
- `Shift+5` changes scanner mode.
- Slow wheel movement remains one product per notch. Continuous input steps by 2, 3, then 5 products as the wheel interval shortens. Pausing or reversing direction resets it to one.
- The physical labels and both display instructions match the new shortcuts.
