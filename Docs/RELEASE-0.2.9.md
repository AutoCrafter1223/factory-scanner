# Item Scanner 0.2.9

## Live first-person placement tuning

- Expanded the visible SML Mods configuration page to seven controls: Pitch, Yaw, Roll, X, Y, Z, and Scale.
- X controls forward/back placement, Y controls left/right placement, and Z controls vertical placement.
- Scale adjusts only the local first-person scanner presentation; world and third-person size remain unchanged.
- All values are read live after returning to gameplay and persisted in `FactoryGame/Configs/ItemScanner.cfg`.

## Verification

- FactoryEditor Development compilation succeeded.
- All 12 ItemScanner automation tests passed.
- Steam and EGS Win64 Shipping compilation, cooking, and packaging succeeded.
- Installed and hash-verified at `D:\SteamLibrary\steamapps\common\Satisfactory\FactoryGame\Mods\ItemScanner-Windows`.
