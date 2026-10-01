#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemScannerEquipmentScreen.generated.h"
class AItemScannerEquipment;
class UTextBlock;
class UVerticalBox;
class UItemScannerDirectionWidget;

/** Entire tree is owned by this UserWidget (and SObjectWidget); no naked UMG Slate rows. */
UCLASS()
class ITEMSCANNER_API UItemScannerEquipmentScreen : public UUserWidget
{
    GENERATED_BODY()
public:
    void Configure(AItemScannerEquipment* Equipment, bool bProduct);
    void Refresh(bool bRebuildResults);
    void AnimateProductStep(int32 Direction, float Duration = 0.12f);
    bool AdvanceProductAnimation(float DeltaSeconds);
protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
private:
    friend class FScannerRenderTest;
    UTextBlock* AddText(UVerticalBox* Parent, const FString& Text, int32 Size, const FLinearColor& Color);
    TWeakObjectPtr<AItemScannerEquipment> ScannerEquipment;
    bool bProductScreen = false;
    float ProductScrollOffset = 0.f;
    float ProductScrollSpeed = 680.f;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> Heading;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> Subheading;
    UPROPERTY(Transient) TObjectPtr<UTextBlock> Footer;
    UPROPERTY(Transient) TObjectPtr<UVerticalBox> Results;
    UPROPERTY(Transient) TArray<TObjectPtr<UTextBlock>> ProductLines;
    UPROPERTY(Transient) TArray<TObjectPtr<UTextBlock>> RowMetrics;
    UPROPERTY(Transient) TArray<TObjectPtr<UItemScannerDirectionWidget>> RowArrows;
};
