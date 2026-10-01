# Factory Scanner 0.2.4 — control and presentation pass

## Controls

- Hold left mouse: open the scanner-only five-way radial menu.
- Move the mouse toward Storage, Production, Conveyor, Logistics, or Mode; release to select.
- Right mouse: scan.
- Shift + wheel: change product with cadence acceleration.
- Plain wheel: untouched, so Satisfactory can switch held equipment.
- R: next result page.

The dynamic mapping context uses an explicit high priority only for scanner-owned chords/buttons. It does not map unmodified wheel input.

## Equipment presentation

- First-person scale: 2.25.
- Offset: centered and lowered to Z=-24.
- Rotation: Pitch 10, Yaw 3, Roll 0, producing a held-tablet view rather than a flat front-facing HUD.
- The stock two-hand portable-miner idle remains the first-pass hand pose. No thumb/finger animation was added. This pass aligns the scanner around the existing hands; a dedicated two-hand IK animation graph remains a separate asset task if in-game running/jumping still shows unacceptable drift.

## Screen behavior

Controls are shown whenever the scanner is newly equipped. After the first scan attempt they are replaced with results or a compact no-matches message. Unequipping and equipping again restores the control guide.
