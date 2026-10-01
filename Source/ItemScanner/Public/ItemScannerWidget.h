#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ComboBoxString.h"
#include "ItemScannerWidget.generated.h"

class UBorder;
class UButton;
class UCheckBox;
class UComboBoxString;
class UEditableTextBox;
class UScrollBox;
class USpinBox;
class UTextBlock;
class UVerticalBox;
class UItemScannerManager;
class UItemScannerTrackingManager;
class UItemScannerWorldSubsystem;
class UTexture2D;
class UItemScannerDirectionWidget;

UCLASS()
class ITEMSCANNER_API UItemScannerWidget final : public UUserWidget
{
    GENERATED_BODY()

public:
    void InitializeScanner(
        UItemScannerManager* InScannerManager,
        UItemScannerTrackingManager* InTrackingManager,
        UItemScannerWorldSubsystem* InHostSubsystem,
        APlayerController* InPlayerController);

    /** Builds the vanilla + installed-mod item catalog once, on the first real open. */
    void PrepareForOpen();

    void ClearScan();

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

private:
    struct FResultRowWidgets
    {
        TWeakObjectPtr<UTextBlock> NameText;
        TWeakObjectPtr<UTextBlock> DistanceText;
        TWeakObjectPtr<UTextBlock> HeightText;
        TWeakObjectPtr<UTextBlock> AmountText;
        TWeakObjectPtr<UItemScannerDirectionWidget> DirectionArrow;
    };

    void BuildInterface();
    bool BuildItemCatalogFromRecipes();
    void RefreshItemOptions(const FString& Filter = FString());
    void BuildResultRows();
    void RefreshResultValues();
    void SetStatus(const FText& Message, const FLinearColor& Color);
    UTexture2D* GetCachedSafeItemIcon(TSubclassOf<class UFGItemDescriptor> ItemClass) const;
    TSubclassOf<class UFGItemDescriptor> FindItemByPath(const FString& ClassPath) const;

    UFUNCTION()
    void HandleScanClicked();

    UFUNCTION()
    void HandleCloseClicked();

    UFUNCTION()
    void HandleSearchTextChanged(const FText& Text);

    UFUNCTION()
    void HandleItemSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION()
    UWidget* HandleGenerateItemWidget(FString Item);

    void HandleTrackingUpdated();
    void HandleTrackingFinished();

    UPROPERTY(Transient)
    TObjectPtr<UItemScannerManager> ScannerManager;

    UPROPERTY(Transient)
    TObjectPtr<UItemScannerTrackingManager> TrackingManager;

    UPROPERTY(Transient)
    TWeakObjectPtr<UItemScannerWorldSubsystem> HostSubsystem;

    UPROPERTY(Transient)
    TWeakObjectPtr<APlayerController> PlayerController;

    UPROPERTY(Transient)
    TArray<TSubclassOf<UFGItemDescriptor>> AvailableItems;

    UPROPERTY(Transient)
    TSubclassOf<UFGItemDescriptor> SelectedItemClass;

    UPROPERTY(Transient)
    TMap<TSubclassOf<UFGItemDescriptor>, TObjectPtr<UTexture2D>> SafeItemIcons;

    UPROPERTY(Transient)
    TObjectPtr<UComboBoxString> ItemComboBox;

    UPROPERTY(Transient)
    TObjectPtr<UEditableTextBox> SearchBox;

    UPROPERTY(Transient)
    TObjectPtr<UCheckBox> StorageCheckBox;

    UPROPERTY(Transient)
    TObjectPtr<UCheckBox> ProductionCheckBox;

    UPROPERTY(Transient)
    TObjectPtr<UCheckBox> ConveyorCheckBox;

    UPROPERTY(Transient)
    TObjectPtr<UCheckBox> LogisticsCheckBox;

    UPROPERTY(Transient)
    TObjectPtr<USpinBox> RangeSpinBox;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> StatusText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> SummaryText;

    UPROPERTY(Transient)
    TObjectPtr<UScrollBox> ResultsScrollBox;

    UPROPERTY(Transient)
    TObjectPtr<UBorder> ResultsBorder;

    TArray<FResultRowWidgets> ResultRows;
    bool bItemCatalogBuilt = false;
    int32 CachedRecipeCount = 0;
};
