# Factory Scanner 0.2.1 — revised CAD / industrial finish

## Design direction

The user's four supplied Satisfactory screenshots are visual references: orange painted shells, dark structural guards and grips, restrained metallic details, and compact high-contrast instrument displays. The supplied CAD remains the source of geometry; no replacement silhouette or external model was substituted.

## Revised source

- `model/scanner.STEP`, SHA-256 `15cf4fcda05d06d5a35d9edc48214b59afa2297ee522b42cd26b53920e6f7c9e`.
- Updated assembly contains the same seven part instances, with newly detailed handle grooves, chamfers and controls.
- Nine game meshes, 18,712 triangles. Original STEP / SolidWorks files remain unchanged.
- Display faces are identified by dimensions and depth, not by the previous model's face indices. New faces: 209 and 337; both are at CAD Z=314 mm.
- The small display and four recessed nameplate labels moved to the new surface depth. UI lies 0.2 mm in front of its CAD surface.
- Existing movement pivots, category toggles, mode switch and dial synchronization are preserved.
- Corrected a CAD-to-Unreal winding error inherited from the first conversion. The coordinate reflection already changes handedness; the redundant triangle flip culled front faces and exposed the rear casing. The new export reverses only OCC-reversed faces. A BaseColor render is retained to verify painted front faces independently of lighting.

## Appearance

- Seven explicit material slots: orange paint, charcoal trim, rubber, metal, ivory marking, safety red, glass.
- Rubber on central grip segments, dark perimeter and control panels, orange enclosure, metal dial collar, ivory category switches and red mode rocker.
- New materials use separate asset names; old materials are retained for recoverability.
- Near-black displays, orange section headings, subdued secondary text, separate result cards and highlighted middle product row. Continuous cyan direction arrows remain unchanged for readability.
- Small printed equipment identifier and mode/dial labels, without changing CAD geometry.

## Scope / verification boundary

Search, connection candidate rules, tracking, no-timeout behavior, crafting costs, saving and input mappings are unchanged. This is an appearance/model update.

Editor D3D12 captures use sample results, not a real loaded save. They verify both screen surfaces and the assembled geometry. Actual first-person grip, movement and FOV remain subject to in-game review; a custom hand animation was not created in this revision.

## Verification

- FactoryEditor Development build passed.
- All eight ItemScanner automation tests passed with D3D12 (zero failures/skips).
- Render regression now checks both live screen regions and the orange front-panel BaseColor, which catches the previous winding defect.
- Assembled render and standalone display previews: `release/preview-0.2.1/`.
- Original STEP hash is unchanged after conversion.
- Steam and Epic Win64 Shipping builds and Windows packaging succeeded; 23 assets cooked.
- 2026-09-28 15:51 KST: installed to `D:\SteamLibrary\steamapps\common\Satisfactory\FactoryGame\Mods\ItemScanner-Windows` with the game closed. All installed package files hash-match the release.
- Previous 0.2.0 was backed up and hash-verified at `release/installed-backup-0.2.0-20260928-155152`. No files were deleted.
