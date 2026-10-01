#pragma once
#include "CoreMinimal.h"
#include "Resources/FGEquipmentDescriptor.h"
#include "FGRecipe.h"
#include "FGSchematic.h"
#include "Unlocks/FGUnlockRecipe.h"
#include "Module/GameWorldModule.h"
#include "ItemScannerContent.generated.h"

UCLASS()
class ITEMSCANNER_API UItemScannerDescriptor : public UFGEquipmentDescriptor
{
    GENERATED_BODY()
public: UItemScannerDescriptor();
};
UCLASS()
class ITEMSCANNER_API UItemScannerRecipe : public UFGRecipe
{
    GENERATED_BODY()
public: UItemScannerRecipe();
};
UCLASS()
class ITEMSCANNER_API UItemScannerRecipeUnlock : public UFGUnlockRecipe
{
    GENERATED_BODY()
public: UItemScannerRecipeUnlock();
};
UCLASS()
class ITEMSCANNER_API UItemScannerSchematic : public UFGSchematic
{
    GENERATED_BODY()
public: UItemScannerSchematic();
};
UCLASS()
class ITEMSCANNER_API UItemScannerGameWorldModule : public UGameWorldModule
{
    GENERATED_BODY()
public: UItemScannerGameWorldModule();
};
