#include "ItemScannerTrackingManager.h"

#include "ItemScannerManager.h"
#include "ItemScannerMath.h"
#include "ItemScannerSettings.h"

#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "Components/SceneComponent.h"

void UItemScannerTrackingManager::Initialize(UItemScannerManager* InScannerManager)
{
    ScannerManager = InScannerManager;
}

void UItemScannerTrackingManager::StartTracking(APlayerController* InPlayerController)
{
    StopTracking(false);

    if (!IsValid(InPlayerController) || !IsValid(ScannerManager))
    {
        return;
    }

    UWorld* World = InPlayerController->GetWorld();
    if (!IsValid(World))
    {
        return;
    }

    const UItemScannerSettings* Settings = GetDefault<UItemScannerSettings>();
    const float UpdateIntervalSeconds = FMath::Max(0.01f, Settings->TrackingUpdateIntervalSeconds);
    PlayerController = InPlayerController;
    TrackingWorld = World;
    bTracking = true;

    UpdateTracking();
    if (!bTracking)
    {
        return;
    }
    World->GetTimerManager().SetTimer(
        TrackingTimer,
        this,
        &UItemScannerTrackingManager::UpdateTracking,
        UpdateIntervalSeconds,
        true);
}

void UItemScannerTrackingManager::StopTracking(bool bClearResults)
{
    if (UWorld* World = TrackingWorld.Get())
    {
        World->GetTimerManager().ClearTimer(TrackingTimer);
    }
    TrackingTimer.Invalidate();

    bTracking = false;
    PlayerController.Reset();
    TrackingWorld.Reset();

    if (bClearResults && IsValid(ScannerManager))
    {
        ScannerManager->ClearResults();
    }
}

void UItemScannerTrackingManager::UpdateTracking()
{
    APlayerController* Controller = PlayerController.Get();
    if (!bTracking || !IsValid(Controller) || !IsValid(ScannerManager))
    {
        StopTracking(true);
        OnTrackingFinished.Broadcast();
        OnTrackingFinishedBP.Broadcast();
        return;
    }

    UWorld* World = Controller->GetWorld();
    if (!IsValid(World) || World != TrackingWorld.Get())
    {
        StopTracking(true);
        OnTrackingFinished.Broadcast();
        OnTrackingFinishedBP.Broadcast();
        return;
    }

    // Keep the scan through temporary pawn loss (e.g. respawn); resume updates
    // when a pawn exists again. Closing the scanner or world teardown clears it.
    if (!IsValid(Controller->GetPawn()))
    {
        return;
    }
    const FVector PlayerLocation = Controller->GetPawn()->GetActorLocation();
    FVector ViewLocation;
    FRotator ViewRotation;
    Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);

    // Only cached actor references and transforms are read here. No actor enumeration or inventory reads.
    for (FItemScannerResult& Result : ScannerManager->GetMutableCachedResults())
    {
        AActor* Target = Result.TargetActor.Get();
        if (!IsValid(Target) || Target->IsActorBeingDestroyed())
        {
            Result.bIsValid = false;
            continue;
        }

        FVector TargetLocation = Target->GetActorLocation();
        if (Result.bConnectionCandidate)
        {
            if (!Result.ConnectionA.IsValid() || !Result.ConnectionB.IsValid())
            {
                Result.bIsValid = false;
                continue;
            }
            TargetLocation = (Result.ConnectionA->GetComponentLocation() + Result.ConnectionB->GetComponentLocation()) * 0.5;
        }
        Result.bIsValid = true;
        Result.CurrentDistanceMeters = FItemScannerMath::CentimetersToMeters(FVector::Distance(PlayerLocation, TargetLocation));
        Result.HeightDifferenceMeters = FItemScannerMath::CentimetersToMeters(TargetLocation.Z - PlayerLocation.Z);
        Result.RelativeYawDegrees = FItemScannerMath::CalculateRelativeYawDegrees(PlayerLocation, ViewRotation.Yaw, TargetLocation);
    }

    OnTrackingUpdated.Broadcast();
    OnTrackingUpdatedBP.Broadcast();
}
