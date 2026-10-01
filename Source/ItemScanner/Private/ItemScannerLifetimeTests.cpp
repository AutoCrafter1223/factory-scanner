#if WITH_DEV_AUTOMATION_TESTS

#include "ItemScannerItemOptionWidget.h"
#include "ItemScannerManager.h"
#include "ItemScannerTrackingManager.h"
#include "ItemScannerWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Engine/Engine.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Misc/AutomationTest.h"
#include "UObject/GarbageCollection.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/SWidget.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FItemScannerOptionLifetimeTest,
    "ItemScanner.UI.OptionLifetimeAcrossGarbageCollection",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FItemScannerOptionLifetimeTest::RunTest(const FString& Parameters)
{
    // Reproduce the old ownership bug without painting freed brush memory.
    TStrongObjectPtr<UWidgetTree> LegacyTree(NewObject<UWidgetTree>());
    UHorizontalBox* LegacyRow = LegacyTree->ConstructWidget<UHorizontalBox>();
    UImage* LegacyIcon = LegacyTree->ConstructWidget<UImage>();
    LegacyRow->AddChildToHorizontalBox(LegacyIcon);
    TWeakObjectPtr<UImage> WeakLegacyIcon = LegacyIcon;
    TSharedPtr<SWidget> LegacySlate = LegacyRow->TakeWidget();
    CollectGarbage(RF_NoFlags);
    TestTrue(TEXT("Legacy Slate row still exists"), LegacySlate.IsValid());
    TestFalse(TEXT("Legacy UImage is collected despite visible Slate row"), WeakLegacyIcon.IsValid());
    LegacySlate.Reset();

    for (int32 Iteration = 0; Iteration < 12; ++Iteration)
    {
        UItemScannerItemOptionWidget* Row = NewObject<UItemScannerItemOptionWidget>();
        Row->Initialize();
        UTexture2D* Texture = UTexture2D::CreateTransient(8, 8);
        Row->SetItem(FText::FromString(TEXT("GC regression item")), Texture);
        TSharedPtr<SWidget> SlateRow = Row->TakeWidget();
        TWeakObjectPtr<UItemScannerItemOptionWidget> WeakRow = Row;
        TWeakObjectPtr<UWidget> WeakIcon = Row->WidgetTree->FindWidget(TEXT("ItemIcon"));
        TWeakObjectPtr<UTexture2D> WeakTexture = Texture;
        for (int32 Pass = 0; Pass < 3; ++Pass)
        {
            CollectGarbage(RF_NoFlags);
            TestTrue(TEXT("Visible option survives GC"), WeakRow.IsValid());
            TestTrue(TEXT("Visible UImage and brush survive GC"), WeakIcon.IsValid());
            TestTrue(TEXT("Icon texture survives GC"), WeakTexture.IsValid());
            SlateRow->SlatePrepass(1.0f);
        }
        SlateRow.Reset();
        CollectGarbage(RF_NoFlags);
        TestFalse(TEXT("Removed option is released, not leaked"), WeakRow.IsValid());
        TestFalse(TEXT("Removed option image is released"), WeakIcon.IsValid());
        TestFalse(TEXT("Unused icon texture is released"), WeakTexture.IsValid());
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FItemScannerPersistentTrackingTest,
    "ItemScanner.Tracking.PersistsUntilScannerClosed",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FItemScannerPersistentTrackingTest::RunTest(const FString& Parameters)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
    Context.SetCurrentWorld(World);
    APlayerController* Controller = World->SpawnActor<APlayerController>();
    APawn* Pawn = World->SpawnActor<APawn>();
    Controller->Possess(Pawn);
    AActor* Target = World->SpawnActor<AActor>();

    TStrongObjectPtr<UItemScannerManager> Manager(NewObject<UItemScannerManager>());
    TStrongObjectPtr<UItemScannerTrackingManager> Tracking(NewObject<UItemScannerTrackingManager>());
    Tracking->Initialize(Manager.Get());
    FItemScannerResult& Result = Manager->GetMutableCachedResults().AddDefaulted_GetRef();
    Result.TargetActor = Target;
    Result.ItemAmount = 123;
    Tracking->StartTracking(Controller);
    for (int32 Step = 0; Step < 240; ++Step)
    {
        ++GFrameCounter;
        World->Tick(LEVELTICK_All, 0.25f);
    }
    TestTrue(TEXT("World advanced past old ten-second timeout"), World->GetTimeSeconds() > 10.0);
    TestTrue(TEXT("Tracking remains active after a minute"), Tracking->IsTracking());
    TestEqual(TEXT("Scan survives after a minute"), Manager->GetCachedResults().Num(), 1);

    Controller->UnPossess();
    ++GFrameCounter;
    World->Tick(LEVELTICK_All, 0.25f);
    TestTrue(TEXT("Temporary pawn loss preserves tracking"), Tracking->IsTracking());
    TestEqual(TEXT("Temporary pawn loss preserves results"), Manager->GetCachedResults().Num(), 1);
    Controller->Possess(Pawn);
    Target->Destroy();
    ++GFrameCounter;
    World->Tick(LEVELTICK_All, 0.25f);
    if (Manager->GetCachedResults().Num() == 1)
    {
        TestFalse(TEXT("Destroyed target is safely marked invalid"), Manager->GetCachedResults()[0].bIsValid);
    }

    TStrongObjectPtr<UItemScannerWidget> Scanner(NewObject<UItemScannerWidget>());
    Scanner->InitializeScanner(Manager.Get(), Tracking.Get(), nullptr, Controller);
    Scanner->ClearScan();
    TestFalse(TEXT("Closing scanner stops tracking"), Tracking->IsTracking());
    TestEqual(TEXT("Closing scanner clears results"), Manager->GetCachedResults().Num(), 0);
    ++GFrameCounter;
    World->Tick(LEVELTICK_All, 1.0f);
    TestEqual(TEXT("Stopped timer cannot repopulate results"), Manager->GetCachedResults().Num(), 0);
    GEngine->DestroyWorldContext(World);
    World->DestroyWorld(false);
    return true;
}

#endif
