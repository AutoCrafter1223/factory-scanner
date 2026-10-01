# SMR registration fields

## Mod page

- **Name:** Factory Scanner
- **Mod reference:** ItemScanner
- **Short description:** Handheld item radar and belt/pipe connection-gap scanner with live direction tracking.
- **Full description:** Paste the contents of `SMR_DESCRIPTION.md`.
- **Icon:** `Resources/Icon128.png`
- **Source link:** https://github.com/AutoCrafter1223/factory-scanner
- **Hidden on creation:** Yes, until the final in-game checks and first public package are complete.

The mod reference is permanent after page creation. It already matches the plugin file, plugin directory, C++ module, and source directory.

## Transparency fields

- **Network activity:** None. The mod does not contact external services.
- **Generative AI:** OpenAI Codex was used for code assistance, debugging, automated-test development, documentation, and release preparation. The author reviewed and tested the resulting changes.

## First public version

- **Recommended version:** 1.0.0
- **Upload file:** the final multi-target Alpakit ZIP produced after the 1.0.0 metadata change
- **Changelog:** Paste the contents of `SMR_RELEASE_NOTES_1.0.0.md`.
- **Stable compatibility:** Works only after the final game test passes
- **Experimental compatibility:** Do not mark Works unless separately tested on that branch

Do not upload `release/ItemScanner-0.3.10-Windows.zip` as the public release. It is the verified internal test package, but its descriptor uses the development version scheme. The public package must use matching `Version`, `VersionName`, and `SemVersion` values for 1.0.0 and must be rebuilt afterward.

## Image files prepared

- `Docs/images/item-search.png`: Item Radar gameplay screenshot
- `Docs/images/connection-check.png`: Connection Check gameplay screenshot
- `Resources/Icon128.png`: square mod icon used by the in-game Mods page and SMR

The two screenshots can be uploaded to a public repository and embedded in the full description after permanent URLs exist. The full description is usable without them.
