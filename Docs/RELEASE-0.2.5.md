# Item Scanner 0.2.5

## Equipment presentation

- Rebuilt all nine equipment meshes from the revised seven-part `scanner.STEP` assembly.
- Re-detected the main and product display faces instead of reusing stale face indices.
- Reversed the first-person pitch so the scanner top tilts away from the camera.
- Kept the stock Portable Miner two-hand pose and deliberately avoided hand IK, skeleton changes, and animation-blueprint replacement.
- Added a small camera-local movement bob (0.35 cm, 0.25 degrees maximum) that fades out while the radial menu is open. Equipment meshes remain collision-free.

## Controls and catalog

- Shift+wheel remains the product selector. Consecutive events now accelerate through 1, 3, 7, 15, and 25 item steps, resetting after a 0.35 second pause.
- The physical rotary animation follows large selection changes more quickly.
- The recipe-derived catalog now rejects build descriptors, non-solid descriptors, and zero-stack descriptors. Buildings are not tracked as items.

## Radial menu

- Replaced the five floating rectangular cards with a code-drawn five-sector circular control.
- The selected sector and centre direction chevron use the Satisfactory-style orange highlight.
- The existing hold-LMB, move, release-to-select interaction is unchanged.
- The dial uses Slate line primitives only and creates no runtime texture or dynamic-material ownership risk.

## Connection check

- Allows compatible disconnected endpoints on different attachments, including junction/pump and merger/splitter combinations.
- Still rejects every same-owner pair, so unused ports on one multi-port junction or splitter do not report each other.
- Connected and snap-only support ports remain excluded.

## Verification and deployment

- FactoryEditor Development and Steam/EGS Shipping compilation succeeded.
- All 12 ItemScanner automation tests passed with zero failures or skips.
- Windows cooking and packaging succeeded with 23 cooked assets.
- Installed and hash-verified at `D:\SteamLibrary\steamapps\common\Satisfactory\FactoryGame\Mods\ItemScanner-Windows`.
- Previous 0.2.4 installation was backed up at `release/installed-backup-0.2.4-20260928-230055`.
