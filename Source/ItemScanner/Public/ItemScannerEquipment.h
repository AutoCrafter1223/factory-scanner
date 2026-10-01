#pragma once
#include "CoreMinimal.h"
#include "Equipment/FGEquipment.h"
#include "ItemScannerTypes.h"
#include "ItemScannerEquipment.generated.h"
class UStaticMeshComponent;
class UWidgetComponent;
class UTextRenderComponent;
class UMaterialInstanceDynamic;
class UEnhancedInputComponent;
class UInputMappingContext;
class UInputAction;
class UItemScannerEquipmentScreen;
class UItemScannerRadialMenu;
class UItemScannerCatalog;
class UItemScannerManager;
class UItemScannerTrackingManager;
struct FInputActionValue;

USTRUCT(BlueprintType)
struct FItemScannerEquipmentState
{
    GENERATED_BODY()
    UPROPERTY(SaveGame) TSubclassOf<UFGItemDescriptor> TargetItem;
    UPROPERTY(SaveGame) EItemScannerMode Mode = EItemScannerMode::ItemSearch;
    UPROPERTY(SaveGame) uint8 Categories = 15;
    UPROPERTY(SaveGame) float RangeMeters = 500.f;
    UPROPERTY(SaveGame) float ConnectionGapMeters = 1.f;
    UPROPERTY(SaveGame) int32 DialSteps = 0;
};

UCLASS()
class ITEMSCANNER_API AItemScannerEquipment : public AFGEquipment
{
    GENERATED_BODY()
public:
    static constexpr int32 ResultsPerPage = 8;
    static FRotator GetModeSelectorRotation(EItemScannerMode Mode);
    AItemScannerEquipment();
    virtual void Equip(AFGCharacterPlayer* Character) override;
    virtual void UnEquip() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void LoadFromItemState_Implementation(const FFGDynamicStruct& ItemState) override;
    virtual FFGDynamicStruct SaveToItemState_Implementation() const override;
    const FItemScannerEquipmentState& GetScannerState() const { return State; }
    UItemScannerManager* GetScanner() const { return Scanner; }
    FText GetProductName(int32 Offset) const;
    FText GetStatus() const { return Status; }
    int32 GetResultPage() const { return ResultPage; }
    bool ShouldShowInstructions() const { return !bHasScannedThisEquip; }
    void Scan();
private:
    friend class FScannerRenderTest;
    friend class FScannerWheelAccelerationTest;
    friend class FScannerRangeCycleTest;
    void InstallInput();
    void RemoveInput();
    void StopScanner();
    bool CanUseInput() const;
    void StepProduct(int32 Step);
    static int32 GetAcceleratedWheelStep(double IntervalSeconds, int32 BurstCount);
    static float GetNextConnectionGapMeters(float Current);
    static float GetNextScanRangeMeters(float Current);
    void WheelProduct(int32 Direction);
    void PreviousProduct() { WheelProduct(-1); }
    void NextProduct() { WheelProduct(1); }
    void MenuPressed();
    void MenuReleased();
    void MenuAxisX(const FInputActionValue& Value);
    void MenuAxisY(const FInputActionValue& Value);
    void OpenRadialMenu();
    void CloseRadialMenu(bool bCommitSelection);
    void UpdateRadialSelection();
    void ToggleCategory(int32 Index);
    void Storage() { ToggleCategory(0); }
    void Production() { ToggleCategory(1); }
    void Conveyor() { ToggleCategory(2); }
    void Logistics() { ToggleCategory(3); }
    void ToggleMode();
    void CycleConnectionGap();
    void CycleScanRange();
    void NextPage();
    void StateChanged();
    void RefreshScreens(bool bRebuildResults);
    void UpdateAttachment();
    UFUNCTION(Server, Reliable) void ServerSetScannerState(FItemScannerEquipmentState NewState);
    UFUNCTION() void OnRep_State();

    UPROPERTY(SaveGame, ReplicatedUsing=OnRep_State) FItemScannerEquipmentState State;
    UPROPERTY(VisibleAnywhere) TObjectPtr<USceneComponent> ModelRoot;
    UPROPERTY(VisibleAnywhere) TObjectPtr<USceneComponent> GeometryRoot;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Body;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> ModeSwitch;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> RotaryDial;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UWidgetComponent> MainDisplay;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UWidgetComponent> ProductDisplay;
    UPROPERTY(Transient) TObjectPtr<UItemScannerEquipmentScreen> ProductScreen;
    UPROPERTY(VisibleAnywhere) TArray<TObjectPtr<UStaticMeshComponent>> StatusLights;
    UPROPERTY(Transient) TArray<TObjectPtr<UMaterialInstanceDynamic>> StatusLightMaterials;
    UPROPERTY(Transient) TObjectPtr<UItemScannerEquipmentScreen> MainScreen;
    UPROPERTY(Transient) TObjectPtr<UItemScannerRadialMenu> RadialMenu;
    UPROPERTY(Transient) TObjectPtr<UEnhancedInputComponent> ScannerInput;
    UPROPERTY(Transient) TObjectPtr<UInputMappingContext> ScannerMapping;
    UPROPERTY(Transient) TArray<TObjectPtr<UInputAction>> ScannerActions;
    UPROPERTY(Transient) TObjectPtr<UItemScannerCatalog> Catalog;
    UPROPERTY(Transient) TObjectPtr<UItemScannerManager> Scanner;
    UPROPERTY(Transient) TObjectPtr<UItemScannerTrackingManager> Tracking;
    TWeakObjectPtr<APlayerController> InputController;
    FBoundMappingContextHandle ScannerMappingHandle;
    FText Status;
    FVector2D RadialDirection=FVector2D::ZeroVector;
    float ScreenUpdateSeconds = 0.f;
    float VisualDialDegrees = 0.f;
    double LastWheelSeconds = -1.0;
    float WheelPlaybackSeconds = 0.f;
    int32 ResultPage = 0;
    int32 LastWheelDirection = 0;
    int32 WheelBurstCount = 0;
    int32 PendingProductSteps = 0;
    bool bScannerActive = false;
    bool bFirstPersonAttached = false;
    bool bRadialMenuOpen = false;
    bool bHasScannedThisEquip = false;
    int32 RadialSelection = INDEX_NONE;
};
