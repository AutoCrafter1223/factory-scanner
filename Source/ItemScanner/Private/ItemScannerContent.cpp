#include "ItemScannerContent.h"
#include "ItemScannerEquipment.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "UObject/ConstructorHelpers.h"

UItemScannerDescriptor::UItemScannerDescriptor()
{
    mUseDisplayNameAndDescription = true;
    mDisplayName = NSLOCTEXT("ItemScanner", "EquipmentName", "Factory Scanner");
    mDescription = NSLOCTEXT("ItemScanner", "EquipmentDescription", "Handheld factory diagnostic tool. Hold left click: control menu. Right click: scan. Shift+wheel: product selection.");
    mStackSize = EStackSize::SS_ONE;
    // This descriptor is a native CDO, not an asset that receives PostLoad.
    // CL502094 Shipping GetStackSize reads this cache directly; unlike the SDK
    // implementation, it does not lazily resolve SS_ONE when the cache is -1.
    mCachedStackSize = 1;
    mForm = EResourceForm::RF_SOLID;
    mEquipmentClass = AItemScannerEquipment::StaticClass();
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(TEXT("/ItemScanner/Equipment/Scanner_Body.Scanner_Body"));
    mConveyorMesh = Mesh.Object;
    // Dedicated CAD-derived icon is imported by the asset pipeline.
    static ConstructorHelpers::FObjectFinder<UTexture2D> Icon(TEXT("/ItemScanner/Equipment/T_ScannerIcon.T_ScannerIcon"));
    mSmallIcon = Icon.Object; mPersistentBigIcon = Icon.Object;
}

UItemScannerRecipe::UItemScannerRecipe()
{
    mDisplayNameOverride = true;
    mDisplayName = NSLOCTEXT("ItemScanner", "EquipmentName", "Factory Scanner");
    mManufactoringDuration = 20.0f;
    mProduct.Add(FItemAmount(UItemScannerDescriptor::StaticClass(), 1));
    // All temporary crafting costs live here, not in input/UI code.
    struct FCost { const TCHAR* Path; int32 Amount; };
    const FCost Costs[] = {
        {TEXT("/Game/FactoryGame/Resource/Equipment/GemstoneScanner/BP_EquipmentDescriptorObjectScanner.BP_EquipmentDescriptorObjectScanner_C"), 1},
        {TEXT("/Game/FactoryGame/Resource/Parts/Rotor/Desc_Rotor.Desc_Rotor_C"), 2},
        {TEXT("/Game/FactoryGame/Resource/Parts/IronPlateReinforced/Desc_IronPlateReinforced.Desc_IronPlateReinforced_C"), 2}
    };
    for (const FCost& Cost : Costs)
    {
        ConstructorHelpers::FClassFinder<UFGItemDescriptor> Item(Cost.Path);
        if (Item.Succeeded()) mIngredients.Add(FItemAmount(Item.Class, Cost.Amount));
    }
    mProducedIn.Add(TSoftClassPtr<UObject>(FSoftObjectPath(TEXT("/Game/FactoryGame/Buildable/-Shared/WorkBench/BP_WorkshopComponent.BP_WorkshopComponent_C"))));
}

UItemScannerRecipeUnlock::UItemScannerRecipeUnlock() { mRecipes.Add(UItemScannerRecipe::StaticClass()); }
UItemScannerSchematic::UItemScannerSchematic()
{
    mType = ESchematicType::EST_Custom;
    mDisplayName = NSLOCTEXT("ItemScanner", "EquipmentName", "Factory Scanner");
    mUnlocks.Add(CreateDefaultSubobject<UItemScannerRecipeUnlock>(TEXT("ScannerRecipeUnlock")));
}
UItemScannerGameWorldModule::UItemScannerGameWorldModule()
{
    bRootModule = true;
    mSchematics.Add(UItemScannerSchematic::StaticClass());
}
