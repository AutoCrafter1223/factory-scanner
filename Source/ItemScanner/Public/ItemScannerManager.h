#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ItemScannerTypes.h"
#include "ItemScannerManager.generated.h"

class APlayerController;
class UFGInventoryComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FItemScannerScanCompleted, const TArray<FItemScannerResult>&);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FItemScannerScanCompletedDynamic);

UCLASS(BlueprintType)
class ITEMSCANNER_API UItemScannerManager final : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Item Scanner")
    bool ExecuteScan(APlayerController* PlayerController, const FItemScannerScanRequest& Request, UPARAM(ref) FText& OutError);

    const TArray<FItemScannerResult>& GetCachedResults() const { return CachedResults; }

    UFUNCTION(BlueprintPure, Category="Item Scanner")
    TArray<FItemScannerResult> GetResultsCopy() const { return CachedResults; }

    UFUNCTION(BlueprintPure, Category="Item Scanner")
    FItemScannerScanRequest GetLastScanRequest() const { return LastRequest; }

    UFUNCTION(BlueprintCallable, Category="Item Scanner")
    void ClearResults();

    const FItemScannerScanRequest& GetLastRequest() const { return LastRequest; }
    TArray<FItemScannerResult>& GetMutableCachedResults() { return CachedResults; }

    FItemScannerScanCompleted OnScanCompleted;

    UPROPERTY(BlueprintAssignable, Category="Item Scanner")
    FItemScannerScanCompletedDynamic OnScanCompletedBP;

private:
    bool ExecuteConnectionScan(APlayerController* Controller, const FItemScannerScanRequest& Request, FText& OutError);
    static bool TryClassifyActor(AActor* Actor, EItemScannerCategory& OutCategory);
    static void CollectInventories(AActor* Actor, TSet<UFGInventoryComponent*>& OutInventories);
    static int64 CountItemInActor(AActor* Actor, TSubclassOf<UFGItemDescriptor> TargetItem);
    static int64 CountItemOnConveyor(AActor* Actor, TSubclassOf<UFGItemDescriptor> TargetItem);
    static FText ResolveActorDisplayName(AActor* Actor);

    UPROPERTY(Transient)
    TArray<FItemScannerResult> CachedResults;

    UPROPERTY(Transient)
    FItemScannerScanRequest LastRequest;
};
