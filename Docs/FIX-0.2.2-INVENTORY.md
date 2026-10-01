# 0.2.2 — native equipment inventory capacity

## Symptom

The Equipment Workshop displays insufficient inventory space for Factory Scanner even with empty slots.

## Cause and evidence

The descriptor is a native C++ class. Its constructor set `mStackSize=SS_ONE` but left the inherited `mCachedStackSize=INDEX_NONE`. Unlike a loaded Blueprint descriptor, its CDO does not receive the asset PostLoad cache initialization.

The CL502094 SDK source's `UFGItemDescriptor::GetStackSize` contains a lazy cache repair. The installed game's Shipping implementation does not: read-only disassembly of the exported function at RVA `0xB53610` shows it returning the CDO field at offset `0x194` directly. There is no resource-settings lookup or cache initialization in that function. These addresses are diagnostic evidence only, not used by the mod.

The installed game's base descriptor constructor at RVA `0xB398D0` also confirms that the field starts at -1 (write at `0xB39A1E`). Neither game binaries nor process memory were modified during diagnosis.

Previous editor tests checked recipe/mesh registration but did not verify the native descriptor's raw inventory capacity. Testing only the public SDK getter would have hidden the missing initialization.

## Fix

Set the scanner's cached capacity explicitly to 1 in its native constructor, alongside SS_ONE and RF_SOLID. This equipment intentionally occupies one inventory slot per item. No inventory-capacity checks are bypassed and no other items or inventory contents are modified. Existing descriptor/recipe class identities remain unchanged.

New regression `ItemScanner.Equipment.NativeInventoryCapacity` checks the raw cache before calling the SDK getter, then solid form, public stack size and the one-item recipe output. The SDK's inventory insertion functions are stubs, so a Shipping crafting retry remains the end-to-end acceptance check.

No CAD, appearance, controls, crafting ingredients, or scanner search behavior changed.

## Verification

FactoryEditor Development compiled successfully. All nine ItemScanner automation tests passed (zero failures/skips), including raw native capacity, recipe output, state fields, GC ownership, direction math, no-timeout tracking and D3D12 display rendering. Results: `release/tests-0.2.2/index.json`.

Steam/Epic Shipping builds and Windows packaging succeeded. At 2026-09-28 16:05 KST, installed 0.2.2 to the user-designated game mod folder after confirming the game had exited. Installation hashes match the release. The previous installation is fully backed up at `release/installed-backup-0.2.1-20260928-160522`. No inventory or save data was edited. Actual Workshop crafting should now be retried in-game.
