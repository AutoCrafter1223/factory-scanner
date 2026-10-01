# Item Scanner 0.2.7

## In-game equipment angle settings

- Added an SML Mods configuration page named `휴대 장비 각도`.
- Added live spin-box controls for first-person Pitch, Yaw, and Roll. Each accepts typing or mouse dragging in the -45 to 45 degree range.
- Values are saved by SML and applied every frame while the scanner is equipped; a game restart is not required after changing them.
- Invalid extreme values are clamped to -89 through 89 degrees before use.
- Position, scale, sway, hand pose, scanning, and input behavior are unchanged.

## Verification

- FactoryEditor Development compilation succeeded.
- All 12 ItemScanner automation tests passed with zero failures or warnings.
- FactoryGame Steam and EGS Win64 Shipping compilation succeeded.
- Windows cooking and packaging succeeded with 23 cooked assets.
- Installed and hash-verified at `D:\SteamLibrary\steamapps\common\Satisfactory\FactoryGame\Mods\ItemScanner-Windows`.
