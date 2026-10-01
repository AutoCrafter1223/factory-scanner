#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemScannerRadialMenu.generated.h"

class AItemScannerEquipment;
class UBorder;
class UTextBlock;
class UItemScannerRadialDialWidget;

/** Mouse-direction radial control shown only while primary fire is held. */
UCLASS()
class ITEMSCANNER_API UItemScannerRadialMenu final : public UUserWidget
{
    GENERATED_BODY()
public:
    void Configure(AItemScannerEquipment* Equipment);
    void SetSelection(int32 Index);
protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
private:
    void RefreshVisuals();
    TWeakObjectPtr<AItemScannerEquipment> ScannerEquipment;
    UPROPERTY(Transient) TArray<TObjectPtr<UTextBlock>> OptionLabels;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> CenterLabel;
    UPROPERTY(Transient) TObjectPtr<UItemScannerRadialDialWidget> Dial;
    int32 SelectedIndex=-1;
};
