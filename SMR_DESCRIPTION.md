# Factory Scanner

Find items in your factory and inspect suspicious gaps between conveyor or pipeline connections with a handheld scanner.

The in-game equipment is named **Factory Scanner**. Craft it in the Equipment Workshop, equip it in a hand slot, choose a mode, and scan when needed.

## Screenshots

### Item Radar

![Factory Scanner locating Iron Plates across nearby factory actors](https://raw.githubusercontent.com/AutoCrafter1223/factory-scanner/main/Docs/images/item-search.png)

### Connection Check

![Factory Scanner reporting disconnected conveyor endpoints](https://raw.githubusercontent.com/AutoCrafter1223/factory-scanner/main/Docs/images/connection-check.png)

## Main features

- Searches for a selected solid item in nearby storage, production buildings, conveyors, and logistics vehicles or stations.
- Shows up to eight results per page with live direction, distance, height difference, category, and item quantity.
- Supports compatible mod items that are registered in active recipes. Buildings and other non-inventory products are excluded from the product list.
- Provides separate **Item Radar** and **Connection Check** modes.
- Connection Check finds compatible, disconnected belt or pipe endpoints that face each other within the selected gap distance.
- Supports 100m, 300m, and 500m radar ranges.
- Supports 0.5m, 1m, and 2m connection-gap ranges; the default is 1m.
- Uses an accelerated but visibly stepped product selector for long item lists.
- Lets players adjust first-person Pitch, Yaw, Roll, X, Y, Z, and Scale from the in-game mod configuration page.
- Performs one world pass only when the player explicitly scans. Live tracking updates cached results instead of repeatedly scanning the world.

## Crafting

The Factory Scanner is automatically available in the Equipment Workshop.

- 1 Object Scanner
- 2 Rotors
- 2 Reinforced Iron Plates
- Crafting time: 20 seconds

## Default controls

- Hold `Left Mouse Button`: open the scanner control menu
- Move the mouse and release `Left Mouse Button`: choose a control-menu option
- `Right Mouse Button`: scan or rescan
- `Shift + Mouse Wheel`: select the previous or next product
- `R`: next result page
- Normal mouse-wheel equipment switching remains available

The control menu can enable or disable Storage, Production, Conveyor, and Logistics searches. It also switches modes and cycles radar or connection-gap distance.

## Connection Check notes

Connection Check reports **suspected gaps**, not guaranteed construction errors. It only pairs compatible, unconnected endpoints on different objects that face each other. Connected ports, snap-only ports, ports on the same object, and isolated unused ports are ignored. Junctions, pumps, mergers, splitters, and wall or floor attachments may participate when their endpoints form a valid pair.

## Compatibility

- Satisfactory game build: `502094` or newer within the compatible game release
- Satisfactory Mod Loader: `3.12.x`
- Windows game client: Steam and Epic
- Dedicated servers: not included in the first release package
- Multiplayer: final host/client acceptance test required before public release

## Privacy and support

This mod does not collect telemetry or contact external services.

- Source and documentation: https://github.com/AutoCrafter1223/factory-scanner
- Bug reports: https://github.com/AutoCrafter1223/factory-scanner/issues
