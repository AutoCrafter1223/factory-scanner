# Verified API notes

The implementation targets the public headers in the SML 3.12.0 starter project rather than inferred class or function names.

- `UFGInventoryComponent::GetNumItems(TSubclassOf<UFGItemDescriptor>)` returns the total count for an inventory, so split slots do not need manual iteration.
- `AFGBuildableConveyorBase` is the common belt/lift parent and exposes `GetConveyorBeltItems`. Each `FConveyorBeltItem` contains one `FInventoryItem`.
- `UModContentRegistry::GetObtainableItemDescriptors` supplies loaded, registered, obtainable descriptor `UClass` objects. `UFGItemDescriptor::GetItemName` and `GetSmallIcon` provide localized names and icons.
- `AFGBuildableStorage` exposes its storage inventory.
- `AFGBuildableManufacturer` exposes separate input and output inventories.
- `AFGBuildableDockingStation`, `AFGBuildableTrainPlatformCargo`, and `AFGBuildableDroneStation` expose their relevant inventories.
- `AFGWheeledVehicle`, `AFGDroneVehicle`, and `AFGFreightWagon` expose cargo inventories and derive through the standard FactoryGame vehicle hierarchy.
- `AFGBuildable::GetBuiltWithDescriptor` and `AFGVehicle::GetBuiltWithDescriptor` provide the normal localized object display name path.

Primary references:

- https://github.com/satisfactorymodding/SatisfactoryModLoader/releases/tag/v3.12.0
- https://github.com/satisfactorymodding/SatisfactoryModLoader/blob/master/Source/FactoryGame/Public/FGInventoryComponent.h
- https://github.com/satisfactorymodding/SatisfactoryModLoader/blob/master/Source/FactoryGame/Public/Buildables/FGBuildableConveyorBase.h
- https://github.com/satisfactorymodding/SatisfactoryModLoader/blob/master/Mods/SML/Source/SML/Public/Registry/ModContentRegistry.h
- https://github.com/satisfactorymodding/SatisfactoryModLoader/blob/master/Source/FactoryGame/Public/Resources/FGItemDescriptor.h
- https://docs.ficsit.app/satisfactory-modding/latest/Development/Cpp/setup.html
