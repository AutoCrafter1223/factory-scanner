#include "ItemScannerManager.h"
#include "ItemScannerConnectionMath.h"
#include "ItemScannerSettings.h"
#include "ItemScannerLocalization.h"
#include "Buildables/FGBuildable.h"
#include "Buildables/FGBuildableConveyorBase.h"
#include "Buildables/FGBuildablePipeline.h"
#include "FGFactoryConnectionComponent.h"
#include "FGPipeConnectionComponent.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"

namespace
{
    struct FPort
    {
        USceneComponent* Component;
        FVector Position, Normal;
        bool bPipe;
    };
    bool Compatible(const FPort& A, const FPort& B)
    {
        // Never pair unused sockets on the same junction/splitter. Different
        // buildables may be attachments on both sides (pump, junction, merger,
        // splitter, wall/floor hole), so do not require a raw belt/pipe actor.
        if (A.bPipe != B.bPipe || A.Component->GetOwner() == B.Component->GetOwner()) return false;
        if (A.bPipe)
        {
            auto* AP = CastChecked<UFGPipeConnectionComponent>(A.Component);
            auto* BP = CastChecked<UFGPipeConnectionComponent>(B.Component);
            return AP->CanConnectTo(BP) && BP->CanConnectTo(AP);
        }
        auto* AC = CastChecked<UFGFactoryConnectionComponent>(A.Component);
        auto* BC = CastChecked<UFGFactoryConnectionComponent>(B.Component);
        return AC->CanConnectTo(BC) && BC->CanConnectTo(AC);
    }
}

bool UItemScannerManager::ExecuteConnectionScan(APlayerController* Controller, const FItemScannerScanRequest& Request, FText& OutError)
{
    if (!IsValid(Controller) || !IsValid(Controller->GetPawn()) || !IsValid(Controller->GetWorld()) ||
        !FMath::IsFinite(Request.RangeMeters) || Request.RangeMeters <= 0)
    {
        OutError = ItemScannerLocalization::Text(TEXT("Player or scan range is not ready."),TEXT("플레이어 또는 스캔 거리가 준비되지 않았습니다."));
        return false;
    }
    const auto* Settings = GetDefault<UItemScannerSettings>();
    LastRequest = Request;
    LastRequest.RangeMeters = FMath::Clamp(Request.RangeMeters, 1.0f, FMath::Max(1.f, Settings->MaxScanRangeMeters));
    const FVector Origin = Controller->GetPawn()->GetActorLocation();
    const double RadiusSq = FMath::Square(LastRequest.RangeMeters * 100.0);
    const float RequestedGapCm = Request.ConnectionGapMeters * 100.f;
    const float Gap = FMath::Clamp(FMath::IsFinite(RequestedGapCm)
        ? RequestedGapCm : Settings->ConnectionCandidateGapCm, 50.f, 200.f);
    LastRequest.ConnectionGapMeters = Gap / 100.f;
    TArray<FPort> Ports;
    // Single world pass, explicit scan only. Filter endpoints, not actor origins (long belts/pipes).
    for (TActorIterator<AFGBuildable> It(Controller->GetWorld()); It; ++It)
    {
        AActor* Actor = *It;
        if (!IsValid(Actor) || Actor->IsActorBeingDestroyed()) continue;
        TInlineComponentArray<UFGFactoryConnectionComponent*> Belts(Actor);
        for (auto* Port : Belts)
        {
            if (!IsValid(Port) || Port->IsConnected() || Port->GetDirection() >= EFactoryConnectionDirection::FCD_SNAP_ONLY) continue;
            const FVector P = Port->GetConnectorLocation();
            if (FVector::DistSquared(P, Origin) <= RadiusSq)
                Ports.Add({Port, P, Port->GetConnectorNormal(), false});
        }
        TInlineComponentArray<UFGPipeConnectionComponent*> Pipes(Actor);
        for (auto* Port : Pipes)
        {
            if (!IsValid(Port) || Port->IsConnected() || Port->GetPipeConnectionType() >= EPipeConnectionType::PCT_SNAP_ONLY) continue;
            const FVector P = Port->GetConnectorLocation();
            if (FVector::DistSquared(P, Origin) <= RadiusSq)
                Ports.Add({Port, P, Port->GetConnectorNormal(), true});
        }
    }
    TMap<FIntVector, TArray<int32>> Cells;
    auto Cell = [Gap](const FVector& P) { return FIntVector(FMath::FloorToInt(P.X/Gap), FMath::FloorToInt(P.Y/Gap), FMath::FloorToInt(P.Z/Gap)); };
    struct FPair { int32 A, B; double Distance; };
    TArray<FPair> Pairs;
    for (int32 I=0; I<Ports.Num(); ++I)
    {
        const FPort& A = Ports[I]; const FIntVector Key = Cell(A.Position);
        for (int32 X=-1; X<=1; ++X) for (int32 Y=-1; Y<=1; ++Y) for (int32 Z=-1; Z<=1; ++Z)
        {
            const auto* Neighbors = Cells.Find(Key+FIntVector(X,Y,Z));
            if (!Neighbors) continue;
            for (int32 J : *Neighbors)
            {
                const FPort& B = Ports[J];
                // Cheap rejections first. At the 2m setting this keeps dense factories
                // from reaching geometry and SDK compatibility checks unnecessarily.
                if (A.bPipe != B.bPipe || A.Component->GetOwner() == B.Component->GetOwner()) continue;
                if (FItemScannerConnectionMath::IsFacingGap(A.Position,A.Normal,B.Position,B.Normal,Gap) && Compatible(A,B))
                    Pairs.Add({I,J,FVector::Distance(A.Position,B.Position)});
            }
        }
        Cells.FindOrAdd(Key).Add(I);
    }
    Pairs.Sort([](const FPair& A, const FPair& B) { return A.Distance < B.Distance; });
    TSet<int32> Used;
    for (const FPair& Pair : Pairs)
    {
        if (Used.Contains(Pair.A) || Used.Contains(Pair.B)) continue;
        Used.Add(Pair.A); Used.Add(Pair.B);
        const FPort& A = Ports[Pair.A]; const FPort& B = Ports[Pair.B];
        FItemScannerResult& R = CachedResults.AddDefaulted_GetRef();
        R.TargetActor = A.Component->GetOwner(); R.ObjectDisplayName = ResolveActorDisplayName(R.TargetActor.Get());
        R.Category = EItemScannerCategory::Conveyor; R.bConnectionCandidate = true; R.bPipeline = A.bPipe;
        R.ConnectionA=A.Component; R.ConnectionB=B.Component; R.GapCentimeters=Pair.Distance;
        R.WorldPositionAtScan=(A.Position+B.Position)*0.5;
        R.DistanceAtScanMeters=FVector::Distance(Origin,R.WorldPositionAtScan)/100;
        R.CurrentDistanceMeters=R.DistanceAtScanMeters;
    }
    CachedResults.StableSort([](const FItemScannerResult& A,const FItemScannerResult& B){return A.DistanceAtScanMeters<B.DistanceAtScanMeters;});
    OnScanCompleted.Broadcast(CachedResults); OnScanCompletedBP.Broadcast();
    return true;
}
