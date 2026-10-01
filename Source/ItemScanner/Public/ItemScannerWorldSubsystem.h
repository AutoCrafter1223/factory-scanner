#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ItemScannerWorldSubsystem.generated.h"

class UInputComponent;
class UItemScannerManager;
class UItemScannerTrackingManager;
class UItemScannerWidget;
class UItemScannerCatalog;

UCLASS()
class ITEMSCANNER_API UItemScannerWorldSubsystem final : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintPure, Category="Item Scanner")
    UItemScannerManager* GetScannerManager() const { return ScannerManager; }

    UFUNCTION(BlueprintPure, Category="Item Scanner")
    UItemScannerTrackingManager* GetTrackingManager() const { return TrackingManager; }

    UItemScannerCatalog* GetCatalog() const { return Catalog; }

    UFUNCTION(BlueprintCallable, Category="Item Scanner")
    void SetScannerVisible(bool bVisible);

    UFUNCTION(BlueprintCallable, Category="Item Scanner")
    void ToggleScannerVisible();

private:
    void TryCreateLocalUI();
    void TryUnlockEquipment();

    UPROPERTY(Transient)
    TObjectPtr<UItemScannerCatalog> Catalog;
    FTimerHandle UnlockTimer;

    UPROPERTY(Transient)
    TObjectPtr<UItemScannerManager> ScannerManager;

    UPROPERTY(Transient)
    TObjectPtr<UItemScannerTrackingManager> TrackingManager;

    UPROPERTY(Transient)
    TObjectPtr<UItemScannerWidget> ScannerWidget;

    UPROPERTY(Transient)
    TObjectPtr<UInputComponent> ScannerInputComponent;

    TWeakObjectPtr<APlayerController> LocalPlayerController;
    FTimerHandle BootstrapTimer;
    bool bPreviousShowMouseCursor = false;
    bool bScannerOwnsInput = false;
};
