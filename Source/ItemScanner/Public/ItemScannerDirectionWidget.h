#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "ItemScannerDirectionWidget.generated.h"

/** Brush-free direction indicator, drawn as a long shaft and arrowhead. */
UCLASS()
class ITEMSCANNER_API UItemScannerDirectionWidget final : public UWidget
{
    GENERATED_BODY()

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
};
