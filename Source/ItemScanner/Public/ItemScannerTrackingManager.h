#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ItemScannerTrackingManager.generated.h"

class APlayerController;
class UItemScannerManager;
class UWorld;

DECLARE_MULTICAST_DELEGATE(FItemScannerTrackingEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FItemScannerTrackingDynamicEvent);

UCLASS(BlueprintType)
class ITEMSCANNER_API UItemScannerTrackingManager final : public UObject
{
    GENERATED_BODY()

public:
    void Initialize(UItemScannerManager* InScannerManager);

    UFUNCTION(BlueprintCallable, Category="Item Scanner")
    void StartTracking(APlayerController* InPlayerController);

    UFUNCTION(BlueprintCallable, Category="Item Scanner")
    void StopTracking(bool bClearResults = true);

    UFUNCTION(BlueprintPure, Category="Item Scanner")
    bool IsTracking() const { return bTracking; }

    FItemScannerTrackingEvent OnTrackingUpdated;
    FItemScannerTrackingEvent OnTrackingFinished;

    UPROPERTY(BlueprintAssignable, Category="Item Scanner")
    FItemScannerTrackingDynamicEvent OnTrackingUpdatedBP;

    UPROPERTY(BlueprintAssignable, Category="Item Scanner")
    FItemScannerTrackingDynamicEvent OnTrackingFinishedBP;

private:
    void UpdateTracking();

    UPROPERTY(Transient)
    TObjectPtr<UItemScannerManager> ScannerManager;

    UPROPERTY(Transient)
    TWeakObjectPtr<APlayerController> PlayerController;

    UPROPERTY(Transient)
    TWeakObjectPtr<UWorld> TrackingWorld;

    FTimerHandle TrackingTimer;
    bool bTracking = false;
};
