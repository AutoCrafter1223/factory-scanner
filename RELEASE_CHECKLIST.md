# Factory Scanner public release checklist

## Prepared and verified

- [x] Permanent mod reference is `ItemScanner` and matches the plugin/module/source names.
- [x] Name, short description, full description, and first-release changelog are prepared.
- [x] Network and generative-AI transparency text is prepared.
- [x] Square mod icon is prepared as `Resources/Icon128.png`.
- [x] Item Radar and Connection Check screenshots are prepared under `Docs/images/`.
- [x] SML dependency is declared as `^3.12.0`.
- [x] Minimum game build is `502094`.
- [x] Steam and Epic Win64 Shipping builds succeeded for internal version 0.3.10 on 2026-10-01.
- [x] Thirteen Item Scanner automated tests passed for internal version 0.3.10 on 2026-10-01.
- [x] Internal 0.3.10 recipe is 1 Object Scanner, 2 Rotors, and 2 Reinforced Iron Plates.
- [x] Current internal package is backed up as `release/ItemScanner-0.3.10-Windows.zip`.

## Final in-game acceptance

- [ ] Craft the scanner in the Equipment Workshop and verify the exact 1/2/2 recipe and 20-second craft time.
- [ ] Equip, unequip, save, load, and restart the game; confirm the scanner remains usable and settings persist.
- [ ] Verify Item Radar with Storage, Production, Conveyor, and Logistics filters individually and together.
- [ ] Verify item quantities, two-column result pages, direction, distance, and height tracking.
- [ ] Verify product selection wraps around and remains visually smooth during fast Shift+wheel input.
- [ ] Verify 100m, 300m, and 500m radar ranges.
- [ ] Verify Connection Check at 0.5m, 1m, and 2m, including belts, pipes, junctions, pumps, mergers/splitters, and wall/floor attachments.
- [ ] Confirm connected ports, snap-only ports, same-object ports, and isolated unused ports are not reported.
- [ ] Confirm first-person transform controls apply and persist with their intended defaults.
- [ ] Check hand alignment, walking/running sway, screen readability, switch directions, LED states, and materials in bright and dark scenes.
- [ ] Check host and client behavior in multiplayer before claiming multiplayer support.
- [ ] Install the final ZIP through a clean Satisfactory Mod Manager profile.

## Public package and SMR

- [ ] Change public version to 1.0.0: `Version: 1`, `VersionName: 1.0.0`, `SemVersion: 1.0.0`.
- [x] Set the author name and source/docs/support URLs.
- [ ] Rebuild editor, Steam Shipping, Epic Shipping, tests, and the combined Alpakit ZIP after metadata changes.
- [ ] Verify the final ZIP includes `Resources/Icon128.png`, both client DLLs, and the pak/ucas/utoc payload.
- [ ] Verify every packaged descriptor reports 1.0.0 and the module manifests retain `BuildId: SML`.
- [ ] Create the SMR page as hidden with permanent reference `ItemScanner`.
- [ ] Upload the final 1.0.0 ZIP and paste `SMR_RELEASE_NOTES_1.0.0.md` into Changelog.
- [ ] Set Stable compatibility to Works only after the corresponding branch test passes.
- [ ] Review the installed page through Satisfactory Mod Manager, then make the page public.
