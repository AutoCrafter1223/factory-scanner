#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "ItemScannerRadialDialWidget.generated.h"

/** Code-drawn seven-sector dial; no texture or dynamic material lifetime risk. */
UCLASS()
class ITEMSCANNER_API UItemScannerRadialDialWidget final : public UWidget
{
    GENERATED_BODY()
public:
    void SetSelection(int32 InSelection);
    int32 GetSelection() const { return Selection; }
protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void ReleaseSlateResources(bool bReleaseChildren) override;
private:
    int32 Selection=INDEX_NONE;
    TSharedPtr<SWidget> DialSlate;
};
