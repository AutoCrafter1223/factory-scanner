#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ItemScannerCatalog.generated.h"
class UFGItemDescriptor;

/** One recipe-derived catalog per world, shared by equipment and debug UI. */
UCLASS()
class ITEMSCANNER_API UItemScannerCatalog : public UObject
{
    GENERATED_BODY()
public:
    bool EnsureBuilt(UObject* WorldContext);
    const TArray<TSubclassOf<UFGItemDescriptor>>& GetItems() const { return Items; }
    static int32 WrapIndex(int32 Index, int32 Count) { return Count > 0 ? ((Index % Count) + Count) % Count : INDEX_NONE; }
private:
    UPROPERTY(Transient) TArray<TSubclassOf<UFGItemDescriptor>> Items;
    FString SortedCulture;
    bool bBuilt = false;
};
