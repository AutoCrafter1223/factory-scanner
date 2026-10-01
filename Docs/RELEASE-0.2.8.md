# Item Scanner 0.2.8

## Visible in-game angle controls

- Fixed the blank Item Scanner page in SML's Mods menu.
- The configuration was registered and persisted correctly in 0.2.7, but its native root section had no visual editor.
- Added a root configuration widget that directly renders Pitch, Yaw, and Roll spin boxes.
- Values can be typed or mouse-dragged from -45 to 45 degrees and are applied after returning to gameplay.

## Verification

- Confirmed from the shipping-game log that SML registered `ItemScannerModConfiguration` and created `FactoryGame/Configs/ItemScanner.cfg`.
- FactoryEditor Development compilation succeeded.
- All 12 ItemScanner automation tests passed, including construction of the root configuration widget.
- Steam and EGS Win64 Shipping compilation, cooking, and packaging succeeded.
- Installed and hash-verified at `D:\SteamLibrary\steamapps\common\Satisfactory\FactoryGame\Mods\ItemScanner-Windows`.
