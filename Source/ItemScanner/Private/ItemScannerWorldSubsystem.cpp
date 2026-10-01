#include "ItemScannerWorldSubsystem.h"

#include "ItemScannerManager.h"
#include "ItemScannerTrackingManager.h"
#include "ItemScannerWidget.h"
#include "ItemScannerCatalog.h"
#include "ItemScannerSettings.h"
#include "ItemScannerContent.h"
#include "FGSchematicManager.h"

#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "FGCharacterPlayer.h"
#include "FGGameState.h"
#include "FGRecipeManager.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "TimerManager.h"

bool UItemScannerWorldSubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
{
    return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UItemScannerWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    Catalog = NewObject<UItemScannerCatalog>(this);

    ScannerManager = NewObject<UItemScannerManager>(this);
    TrackingManager = NewObject<UItemScannerTrackingManager>(this);
    TrackingManager->Initialize(ScannerManager);
}

void UItemScannerWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    if (InWorld.GetNetMode() != NM_Client)
        InWorld.GetTimerManager().SetTimer(UnlockTimer, this, &UItemScannerWorldSubsystem::TryUnlockEquipment, 1.f, true);

    if (InWorld.GetNetMode() == NM_DedicatedServer || !GetDefault<UItemScannerSettings>()->bEnableDebugHUD)
    {
        return;
    }

    InWorld.GetTimerManager().SetTimer(
        BootstrapTimer,
        this,
        &UItemScannerWorldSubsystem::TryCreateLocalUI,
        0.5f,
        true,
        0.0f);
}

void UItemScannerWorldSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(BootstrapTimer);
        World->GetTimerManager().ClearTimer(UnlockTimer);
    }

    if (IsValid(TrackingManager))
    {
        TrackingManager->StopTracking(true);
    }

    if (APlayerController* Controller = LocalPlayerController.Get())
    {
        if (IsValid(ScannerInputComponent))
        {
            Controller->PopInputComponent(ScannerInputComponent);
        }
        if (bScannerOwnsInput)
        {
            Controller->SetShowMouseCursor(bPreviousShowMouseCursor);
            Controller->SetInputMode(FInputModeGameOnly());
            bScannerOwnsInput = false;
        }
    }

    if (IsValid(ScannerWidget))
    {
        ScannerWidget->RemoveFromParent();
    }

    ScannerInputComponent = nullptr;
    ScannerWidget = nullptr;
    LocalPlayerController.Reset();
    Super::Deinitialize();
}

void UItemScannerWorldSubsystem::TryUnlockEquipment()
{
    UWorld* World = GetWorld();
    if (!World || !World->GetGameState<AFGGameState>()) return;
    AFGSchematicManager* Schematics = AFGSchematicManager::Get(World);
    AFGRecipeManager* Recipes = AFGRecipeManager::Get(World);
    if (!IsValid(Schematics) || !IsValid(Recipes) || Recipes->GetAllRecipes().IsEmpty()) return;
    if (!Schematics->IsSchematicPurchased(UItemScannerSchematic::StaticClass()))
        Schematics->GiveAccessToSchematic(UItemScannerSchematic::StaticClass(), nullptr);
    World->GetTimerManager().ClearTimer(UnlockTimer);
}

void UItemScannerWorldSubsystem::SetScannerVisible(bool bVisible)
{
    if (!IsValid(ScannerWidget))
    {
        return;
    }

    if (bVisible)
    {
        ScannerWidget->PrepareForOpen();
    }
    else
    {
        ScannerWidget->ClearScan();
    }

    APlayerController* Controller = LocalPlayerController.Get();
    const bool bWasVisible = ScannerWidget->GetVisibility() == ESlateVisibility::Visible;
    ScannerWidget->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

    if (!IsValid(Controller))
    {
        return;
    }

    if (bVisible)
    {
        if (!bWasVisible || !bScannerOwnsInput)
        {
            bPreviousShowMouseCursor = Controller->bShowMouseCursor;
        }
        bScannerOwnsInput = true;
        Controller->SetShowMouseCursor(true);
        FInputModeGameAndUI InputMode;
        InputMode.SetWidgetToFocus(ScannerWidget->TakeWidget());
        InputMode.SetHideCursorDuringCapture(false);
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        Controller->SetInputMode(InputMode);
    }
    else if (bScannerOwnsInput)
    {
        Controller->SetShowMouseCursor(bPreviousShowMouseCursor);
        Controller->SetInputMode(FInputModeGameOnly());
        bScannerOwnsInput = false;
    }
}

void UItemScannerWorldSubsystem::ToggleScannerVisible()
{
    if (IsValid(ScannerWidget))
    {
        SetScannerVisible(ScannerWidget->GetVisibility() != ESlateVisibility::Visible);
    }
}

void UItemScannerWorldSubsystem::TryCreateLocalUI()
{
    UWorld* World = GetWorld();
    if (!IsValid(World))
    {
        return;
    }

    APlayerController* Controller = World->GetFirstPlayerController();
    AFGCharacterPlayer* CharacterPlayer = IsValid(Controller) ? Cast<AFGCharacterPlayer>(Controller->GetPawn()) : nullptr;
    AFGGameState* GameState = World->GetGameState<AFGGameState>();
    AFGRecipeManager* RecipeManager = AFGRecipeManager::Get(World);
    if (!IsValid(Controller) || !Controller->IsLocalController() ||
        !IsValid(CharacterPlayer) || !IsValid(GameState) || !IsValid(RecipeManager) ||
        RecipeManager->GetAllRecipes().IsEmpty())
    {
        return;
    }

    LocalPlayerController = Controller;
    ScannerWidget = CreateWidget<UItemScannerWidget>(Controller, UItemScannerWidget::StaticClass());
    if (!IsValid(ScannerWidget))
    {
        return;
    }

    ScannerWidget->InitializeScanner(ScannerManager, TrackingManager, this, Controller);
    ScannerWidget->SetVisibility(ESlateVisibility::Collapsed);
    ScannerWidget->AddToViewport(1000);

    ScannerInputComponent = NewObject<UInputComponent>(Controller, TEXT("ItemScannerInputComponent"));
    ScannerInputComponent->RegisterComponent();
    ScannerInputComponent->BindKey(EKeys::F7, IE_Pressed, this, &UItemScannerWorldSubsystem::ToggleScannerVisible);
    Controller->PushInputComponent(ScannerInputComponent);

    World->GetTimerManager().ClearTimer(BootstrapTimer);
    SetScannerVisible(true);
}
