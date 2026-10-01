#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "ItemScannerCatalog.h"
#include "ItemScannerConnectionMath.h"
#include "ItemScannerEquipment.h"
#include "ItemScannerEquipmentScreen.h"
#include "ItemScannerRadialMenu.h"
#include "ItemScannerRadialDialWidget.h"
#include "ItemScannerContent.h"
#include "ItemScannerSettings.h"
#include "ItemScannerModConfig.h"
#include "Configuration/Properties/ConfigPropertyFloat.h"
#include "Configuration/RootConfigValueHolder.h"
#include "Blueprint/WidgetTree.h"
#include "Components/StaticMeshComponent.h"
#include "Components/Border.h"
#include "Components/WidgetComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/GarbageCollection.h"
#include "Widgets/SWidget.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FScannerInventoryDescriptorTest,"ItemScanner.Equipment.NativeInventoryCapacity",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FScannerInventoryDescriptorTest::RunTest(const FString& Parameters)
{
    // Check raw cache BEFORE invoking the SDK getter, which masks this Shipping bug
    // by lazily repairing INDEX_NONE. Shipping only returns the cached field.
    auto* Descriptor=GetDefault<UItemScannerDescriptor>();
    const auto* CachedSize=FindFProperty<FIntProperty>(UFGItemDescriptor::StaticClass(),TEXT("mCachedStackSize"));
    if (!TestNotNull(TEXT("Inventory stack cache exists"),CachedSize)) return false;
    TestEqual(TEXT("Native descriptor immediately has one-item slot capacity"),CachedSize->GetPropertyValue_InContainer(Descriptor),1);
    TestEqual(TEXT("Inventory item is solid"),Descriptor->Form(),EResourceForm::RF_SOLID);
    TestEqual(TEXT("Public stack-size API reports one"),UFGItemDescriptor::GetStackSize(UItemScannerDescriptor::StaticClass()),1);
    const auto& Products=GetDefault<UItemScannerRecipe>()->GetProducts();
    if (TestEqual(TEXT("One recipe output"),Products.Num(),1))
    {
        TestEqual(TEXT("Recipe outputs scanner"),Products[0].ItemClass.Get(),UItemScannerDescriptor::StaticClass());
        TestEqual(TEXT("Recipe output fits one empty slot"),Products[0].Amount,1);
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FScannerFirstPersonScaleTest,"ItemScanner.Equipment.FirstPersonPresentation",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FScannerFirstPersonScaleTest::RunTest(const FString& Parameters)
{
    const auto* Settings=GetDefault<UItemScannerSettings>();
    TestEqual(TEXT("First-person scanner uses the approved readable scale"),Settings->FirstPersonScale,1.5f);
    TestEqual(TEXT("First-person X default"),Settings->FirstPersonOffset.X,3.0);
    TestEqual(TEXT("First-person Y default"),Settings->FirstPersonOffset.Y,-9.0);
    TestEqual(TEXT("First-person Z default"),Settings->FirstPersonOffset.Z,10.0);
    TestEqual(TEXT("First-person pitch default"),Settings->FirstPersonRotation.Pitch,-3.0);
    TestEqual(TEXT("First-person yaw default"),Settings->FirstPersonRotation.Yaw,-11.0);
    TestEqual(TEXT("First-person roll default"),Settings->FirstPersonRotation.Roll,-4.0);
    TestEqual(TEXT("Third-person/world size remains unchanged"),Settings->EquipmentScale,1.f);
    auto* Template=GetDefault<UItemScannerModConfiguration>()->RootSection.Get();
    auto* Holder=NewObject<URootConfigValueHolder>();
    // Mirror SML's private UpdateWrappedValue construction without modifying SML.
    auto* Root=NewObject<UConfigPropertySection>(Holder,Template->GetClass(),NAME_None,RF_NoFlags,Template);
    const TCHAR* Keys[]={TEXT("ObjectScannerX"),TEXT("ObjectScannerY"),TEXT("ObjectScannerZ"),TEXT("ObjectScannerPitch"),TEXT("ObjectScannerYaw"),TEXT("ObjectScannerRoll"),TEXT("ObjectScannerScale")};
    const float Expected[]={3.f,-9.f,10.f,-3.f,-11.f,-4.f,1.5f};
    for (int32 I=0;I<7;++I)
    {
        auto* Property=Cast<UItemScannerAngleConfigProperty>(Root->SectionProperties.FindRef(Keys[I]).Get());
        if (!TestNotNull(Keys[I],Property)) continue;
        TestEqual(TEXT("Live config starts with approved default"),Property->Value,Expected[I]);
        TestTrue(TEXT("Live property belongs to save-aware root"),Property->IsIn(Root));
        TestTrue(TEXT("Live property is not shared with defaults"),Property!=Template->SectionProperties.FindRef(Keys[I]).Get());
    }
    auto* ScaleProperty=Cast<UItemScannerAngleConfigProperty>(Root->SectionProperties.FindRef(TEXT("ObjectScannerScale")).Get());
    if (ScaleProperty) TestEqual(TEXT("Scale fine adjustment survives config duplication"),ScaleProperty->StepSize,0.05f);
    auto* EditedX=Cast<UItemScannerAngleConfigProperty>(Root->SectionProperties.FindRef(TEXT("ObjectScannerX")).Get());
    if (EditedX)
    {
        EditedX->Value=4.f;
        auto* Saved=Root->Serialize(GetTransientPackage());
        auto* Reloaded=NewObject<UConfigPropertySection>(Holder,Template->GetClass(),NAME_None,RF_NoFlags,Template);
        Reloaded->Deserialize(Saved);
        auto* ReloadedX=Cast<UItemScannerAngleConfigProperty>(Reloaded->SectionProperties.FindRef(TEXT("ObjectScannerX")).Get());
        if (ReloadedX) TestEqual(TEXT("User fine adjustment survives serialization and reload"),ReloadedX->Value,4.f);
        auto* DefaultX=Cast<UItemScannerAngleConfigProperty>(Template->SectionProperties.FindRef(TEXT("ObjectScannerX")).Get());
        if (DefaultX) TestEqual(TEXT("User edit does not overwrite reset default"),DefaultX->Value,3.f);
    }
    for (auto Mode : {EItemScannerMode::ItemSearch,EItemScannerMode::ConnectionCheck})
    {
        const FVector Index=FVector(-1.07207,0,0.90037).GetSafeNormal();
        const FVector Aim=FVector(Mode==EItemScannerMode::ItemSearch?1.15:-1.15,0,2.89135).GetSafeNormal();
        TestTrue(TEXT("White selector index points at mode label"),FVector::DotProduct(AItemScannerEquipment::GetModeSelectorRotation(Mode).RotateVector(Index),Aim)>0.9999);
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FScannerWheelAccelerationTest,"ItemScanner.Equipment.WheelAcceleration",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FScannerWheelAccelerationTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("Deliberate wheel input stays precise"),AItemScannerEquipment::GetAcceleratedWheelStep(0.50,1),1);
    TestEqual(TEXT("Second wheel event queues two visible steps"),AItemScannerEquipment::GetAcceleratedWheelStep(0.20,2),2);
    TestEqual(TEXT("Third wheel event queues three visible steps"),AItemScannerEquipment::GetAcceleratedWheelStep(0.15,3),3);
    TestEqual(TEXT("Sustained wheel burst queues five visible steps"),AItemScannerEquipment::GetAcceleratedWheelStep(0.12,5),5);
    TestEqual(TEXT("Long wheel burst queues eight visible steps"),AItemScannerEquipment::GetAcceleratedWheelStep(0.08,7),8);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FScannerRangeCycleTest,"ItemScanner.Equipment.RangeCycles",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FScannerRangeCycleTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("Gap 0.5m advances to 1m"),AItemScannerEquipment::GetNextConnectionGapMeters(0.5f),1.f);
    TestEqual(TEXT("Gap 1m advances to 2m"),AItemScannerEquipment::GetNextConnectionGapMeters(1.f),2.f);
    TestEqual(TEXT("Gap 2m wraps to 0.5m"),AItemScannerEquipment::GetNextConnectionGapMeters(2.f),0.5f);
    TestEqual(TEXT("Range 100m advances to 300m"),AItemScannerEquipment::GetNextScanRangeMeters(100.f),300.f);
    TestEqual(TEXT("Range 300m advances to 500m"),AItemScannerEquipment::GetNextScanRangeMeters(300.f),500.f);
    TestEqual(TEXT("Range 500m wraps to 100m"),AItemScannerEquipment::GetNextScanRangeMeters(500.f),100.f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FScannerControlsTest,"ItemScanner.Equipment.CatalogWrapAndConnectionGeometry",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FScannerControlsTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("Previous wraps to end"),UItemScannerCatalog::WrapIndex(-1,5),4);
    TestEqual(TEXT("Next wraps to start"),UItemScannerCatalog::WrapIndex(5,5),0);
    TestEqual(TEXT("Empty catalog has no item"),UItemScannerCatalog::WrapIndex(1,0),INDEX_NONE);
    TestEqual(TEXT("Single item remains selected"),UItemScannerCatalog::WrapIndex(-1,1),0);
    const FVector A(0,0,0), Normal(1,0,0);
    TestTrue(TEXT("Facing 7cm gap"),FItemScannerConnectionMath::IsFacingGap(A,Normal,FVector(7,0,0),-Normal,30));
    TestTrue(TEXT("Coincident disconnected endpoints"),FItemScannerConnectionMath::IsFacingGap(A,Normal,A,-Normal,30));
    TestFalse(TEXT("Parallel ports not faults"),FItemScannerConnectionMath::IsFacingGap(A,Normal,FVector(7,0,0),Normal,30));
    TestFalse(TEXT("Sideways unused ports not faults"),FItemScannerConnectionMath::IsFacingGap(A,Normal,FVector(0,7,0),-Normal,30));
    TestFalse(TEXT("Distant endpoints not faults"),FItemScannerConnectionMath::IsFacingGap(A,Normal,FVector(100,0,0),-Normal,30));
    TestTrue(TEXT("Two metre option includes a 2m facing gap"),FItemScannerConnectionMath::IsFacingGap(A,Normal,FVector(200,0,0),-Normal,200));
    TestFalse(TEXT("Two metre option excludes anything beyond 2m"),FItemScannerConnectionMath::IsFacingGap(A,Normal,FVector(201,0,0),-Normal,200));
    TestFalse(TEXT("Backwards gap not fault"),FItemScannerConnectionMath::IsFacingGap(A,Normal,FVector(-7,0,0),-Normal,30));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FScannerStateSaveTest,"ItemScanner.Equipment.StateFieldsSaveRoundTrip",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FScannerStateSaveTest::RunTest(const FString& Parameters)
{
    FItemScannerEquipmentState State;
    State.Categories=5; State.Mode=EItemScannerMode::ConnectionCheck; State.RangeMeters=735.f; State.ConnectionGapMeters=2.f;
    State.DialSteps=11; State.TargetItem=UItemScannerDescriptor::StaticClass();
    // SDK FFGDynamicStruct::InitializeAsRaw is a stub. Test our reflected payload,
    // not the absent shipping inventory lifecycle implementation.
    TArray<uint8> Bytes;
    FMemoryWriter Writer(Bytes);
    FObjectAndNameAsStringProxyArchive SaveArchive(Writer,false); SaveArchive.ArIsSaveGame=true;
    FItemScannerEquipmentState::StaticStruct()->SerializeItem(SaveArchive,&State,nullptr);
    TestFalse(TEXT("Save archive valid"),SaveArchive.IsError());
    FMemoryReader Reader(Bytes);
    FObjectAndNameAsStringProxyArchive LoadArchive(Reader,true); LoadArchive.ArIsSaveGame=true;
    FItemScannerEquipmentState Restored;
    FItemScannerEquipmentState::StaticStruct()->SerializeItem(LoadArchive,&Restored,nullptr);
    TestFalse(TEXT("Load archive valid"),LoadArchive.IsError());
    const auto* Value=&Restored;
    TestEqual(TEXT("Categories persisted"),Value->Categories,State.Categories);
    TestEqual(TEXT("Mode persisted"),Value->Mode,State.Mode);
    TestEqual(TEXT("Target persisted"),Value->TargetItem.Get(),State.TargetItem.Get());
    TestEqual(TEXT("Range persisted"),Value->RangeMeters,State.RangeMeters);
    TestEqual(TEXT("Connection gap persisted"),Value->ConnectionGapMeters,State.ConnectionGapMeters);
    TestEqual(TEXT("Dial position persisted"),Value->DialSteps,State.DialSteps);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FScannerAssetsTest,"ItemScanner.Equipment.CADAssetsAndRecipe",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FScannerAssetsTest::RunTest(const FString& Parameters)
{
    auto* Equipment=GetMutableDefault<AItemScannerEquipment>();
    TInlineComponentArray<UStaticMeshComponent*> Meshes(Equipment);
    TestEqual(TEXT("Four CAD meshes and four status lights"),Meshes.Num(),8);
    for (auto* Mesh : Meshes)
    {
        TestNotNull(*Mesh->GetName(),Mesh->GetStaticMesh().Get());
        if (Mesh->GetStaticMesh()) TestTrue(TEXT("Mesh has triangles"),Mesh->GetStaticMesh()->GetNumTriangles(0)>0);
        TestEqual(TEXT("No equipment collision"),Mesh->GetCollisionEnabled(),ECollisionEnabled::NoCollision);
    }
    TInlineComponentArray<UWidgetComponent*> Screens(Equipment);
    TestEqual(TEXT("Results and selected-product screens"),Screens.Num(),2);
    for (auto* Screen : Screens) TestEqual(TEXT("Screens are worldspace"),Screen->GetWidgetSpace(),EWidgetSpace::World);
    const auto* Recipe=GetDefault<UItemScannerRecipe>();
    TestNotNull(TEXT("Recipe default"),Recipe);
    TestEqual(TEXT("Three valid crafting ingredients"),Recipe->GetIngredients().Num(),3);
    const TCHAR* ExpectedIngredients[] = {
        TEXT("BP_EquipmentDescriptorObjectScanner_C"), TEXT("Desc_Rotor_C"), TEXT("Desc_IronPlateReinforced_C")
    };
    const int32 ExpectedAmounts[] = {1, 2, 2};
    for (int32 Index=0; Index<Recipe->GetIngredients().Num() && Index<3; ++Index)
    {
        const auto& Ingredient=Recipe->GetIngredients()[Index];
        TestEqual(TEXT("Crafting ingredient class"),Ingredient.ItemClass->GetName(),FString(ExpectedIngredients[Index]));
        TestEqual(TEXT("Crafting ingredient amount"),Ingredient.Amount,ExpectedAmounts[Index]);
    }
    TestEqual(TEXT("Exactly one product"),Recipe->GetProducts().Num(),1);
    TestTrue(TEXT("Native SML root module"),GetDefault<UItemScannerGameWorldModule>()->bRootModule);
    const auto* GameInstanceModule=GetDefault<UItemScannerGameInstanceModule>();
    TestTrue(TEXT("Native SML game-instance root module"),GameInstanceModule->bRootModule);
    TestTrue(TEXT("In-game angle configuration registered"),GameInstanceModule->ModConfigurations.Contains(UItemScannerModConfiguration::StaticClass()));
    const auto* ModConfig=GetDefault<UItemScannerModConfiguration>();
    TestNotNull(TEXT("In-game angle configuration root"),ModConfig->RootSection.Get());
    if (ModConfig->RootSection)
    {
        const auto* AngleSection=Cast<UItemScannerAngleConfigSection>(ModConfig->RootSection.Get());
        TestNotNull(TEXT("Configuration root has an in-game visual editor"),AngleSection);
        TestEqual(TEXT("Rotation, position, and scale controls"),ModConfig->RootSection->SectionProperties.Num(),7);
        const auto* Pitch=Cast<UItemScannerAngleConfigProperty>(ModConfig->RootSection->SectionProperties.FindRef(TEXT("ObjectScannerPitch")).Get());
        TestNotNull(TEXT("Pitch control has an in-game numeric editor"),Pitch);
        if (Pitch)
        {
            auto* Host=NewObject<UItemScannerAngleConfigEditorWidget>(); Host->Initialize();
            Host->InitializeProperty(const_cast<UItemScannerAngleConfigProperty*>(Pitch));
            TSharedPtr<SWidget> AngleEditor=Host->TakeWidget();
            TestTrue(TEXT("Angle editor builds its Slate controls"),AngleEditor.IsValid());
        }
        if (AngleSection)
        {
            auto* SectionHost=NewObject<UItemScannerAngleConfigSectionWidget>(); SectionHost->Initialize();
            SectionHost->InitializeSection(const_cast<UItemScannerAngleConfigSection*>(AngleSection));
            TSharedPtr<SWidget> SectionSlate=SectionHost->TakeWidget();
            TestTrue(TEXT("Configuration root renders all controls"),SectionSlate.IsValid());
        }
    }
    TestEqual(TEXT("Equipment descriptor"),GetDefault<UItemScannerDescriptor>()->mEquipmentClass.Get(),AItemScannerEquipment::StaticClass());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FScannerScreenLifetimeTest,"ItemScanner.UI.EquipmentScreenLifetime",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FScannerScreenLifetimeTest::RunTest(const FString& Parameters)
{
    for (int32 Pass=0;Pass<12;++Pass)
    {
        auto* Screen=NewObject<UItemScannerEquipmentScreen>(); Screen->Initialize();
        Screen->Configure(nullptr,(Pass%2)==0);
        TSharedPtr<SWidget> Slate=Screen->TakeWidget();
        TWeakObjectPtr<UItemScannerEquipmentScreen> Weak=Screen;
        TWeakObjectPtr<UWidget> Root=Screen->WidgetTree->RootWidget;
        CollectGarbage(RF_NoFlags);
        TestTrue(TEXT("Visible screen owns its widget tree across GC"),Weak.IsValid() && Root.IsValid());
        Slate->SlatePrepass(1.f); Slate.Reset(); CollectGarbage(RF_NoFlags);
        TestFalse(TEXT("Closed screen released"),Weak.IsValid());
        TestFalse(TEXT("Closed screen tree released"),Root.IsValid());
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FScannerRadialMenuTest,"ItemScanner.UI.RadialControlMenu",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FScannerRadialMenuTest::RunTest(const FString& Parameters)
{
    auto* Menu=NewObject<UItemScannerRadialMenu>(); Menu->Initialize();
    Menu->Configure(GetMutableDefault<AItemScannerEquipment>());
    TSharedPtr<SWidget> Slate=Menu->TakeWidget();
    TArray<UWidget*> Widgets; Menu->WidgetTree->GetAllWidgets(Widgets);
    const int32 Dials=Widgets.FilterByPredicate([](const UWidget* Widget){ return Widget->IsA<UItemScannerRadialDialWidget>(); }).Num();
    TestEqual(TEXT("Radial menu owns one code-drawn seven-sector dial"),Dials,1);
    Menu->SetSelection(0); Menu->SetSelection(4); Menu->SetSelection(6); Menu->SetSelection(INDEX_NONE);
    TestTrue(TEXT("Radial menu Slate tree exists"),Slate.IsValid());
    return true;
}
#endif
