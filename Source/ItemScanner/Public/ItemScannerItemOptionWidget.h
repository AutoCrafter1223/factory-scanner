#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemScannerItemOptionWidget.generated.h"

class UTexture2D;

/** Combo-box entries live in Slate, outside the scanner's UMG tree.
 * UUserWidget supplies an SObjectWidget GC bridge for the entire row tree. */
UCLASS()
class ITEMSCANNER_API UItemScannerItemOptionWidget final : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetItem(const FText& InName, UTexture2D* InIcon);

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;

private:
    UPROPERTY(Transient)
    FText ItemName;

    UPROPERTY(Transient)
    TObjectPtr<UTexture2D> ItemIcon;
};
