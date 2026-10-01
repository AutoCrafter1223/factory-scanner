#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/App.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "ItemScannerEquipment.h"
#include "ItemScannerEquipmentScreen.h"
#include "ItemScannerManager.h"
#include "ItemScannerContent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Slate/WidgetRenderer.h"
#include "ImageUtils.h"
#include "Serialization/BufferArchive.h"
#include "UObject/StrongObjectPtr.h"
#include "RenderingThread.h"
#include "Components/TextBlock.h"
#include "Components/WidgetComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SceneCapture2D.h"
#include "ShaderCompiler.h"
#include "AssetCompilingManager.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/StaticMeshComponent.h"
#include "MaterialShared.h"
#include "UObject/GarbageCollection.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FScannerRenderTest,"ItemScanner.Render.EquipmentDisplays",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FScannerRenderTest::RunTest(const FString& Parameters)
{
    if (!FApp::CanEverRender()) { AddInfo(TEXT("Visual check requires non-NullRHI run.")); return true; }
    UWorld* World=UWorld::CreateWorld(EWorldType::Game,false);
    GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
    AItemScannerEquipment* Equipment=World->SpawnActor<AItemScannerEquipment>();
    Equipment->Scanner=NewObject<UItemScannerManager>(Equipment);
    Equipment->Status=FText::FromString(TEXT("TRACKING"));
    Equipment->bHasScannedThisEquip=true;
    Equipment->State.TargetItem=UItemScannerDescriptor::StaticClass();
    for (int32 I=0;I<9;++I)
    {
        auto& R=Equipment->Scanner->GetMutableCachedResults().AddDefaulted_GetRef();
        R.ObjectDisplayName=FText::FromString(I==0?TEXT("컨베이어 벨트 Mk.5"):I==1?TEXT("제작기"):TEXT("Industrial Storage"));
        R.CurrentDistanceMeters=42+I*37; R.HeightDifferenceMeters=I*7-4; R.RelativeYawDegrees=I*61-35;
        R.ItemAmount=100*(I+1); R.Category=static_cast<EItemScannerCategory>(I%4);
    }
    const FString Out=TEXT("D:/codex/item scanner/release/preview-0.3.9/");
    IFileManager::Get().MakeDirectory(*Out,true);
    for (int32 I=0;I<2;++I)
    {
        const bool bProduct=false;
        if (I==1) Equipment->State.Mode=EItemScannerMode::ConnectionCheck;
        TStrongObjectPtr<UItemScannerEquipmentScreen> Screen(NewObject<UItemScannerEquipmentScreen>(Equipment));
        Screen->Initialize(); Screen->Configure(Equipment,bProduct);
        auto Slate=Screen->TakeWidget(); Screen->Refresh(true);
        CollectGarbage(RF_NoFlags);
        Screen->Refresh(false);
        FWidgetRenderer* Renderer=new FWidgetRenderer(false);
        TStrongObjectPtr<UTextureRenderTarget2D> Target(Renderer->DrawWidget(Slate,FVector2D(768,826)));
        FlushRenderingCommands();
        FBufferArchive Bytes;
        TestTrue(TEXT("Display rendered as PNG"),FImageUtils::ExportRenderTarget2DAsPNG(Target.Get(),Bytes));
        TestTrue(TEXT("Preview saved"),FFileHelper::SaveArrayToFile(Bytes,*(Out+(I==0?TEXT("main.png"):TEXT("connection.png")))));
        BeginCleanup(Renderer);
        if (I==0) { Equipment->MainScreen=Screen.Get(); Equipment->MainDisplay->SetWidget(Screen.Get()); }
    }
    Equipment->SetActorHiddenInGame(false);
    Equipment->State.Mode=EItemScannerMode::ItemSearch;
    Equipment->MainScreen->Refresh(true);
    TestEqual(TEXT("Eight visible result metrics"),Equipment->MainScreen->RowMetrics.Num(),8);
    Equipment->ModeSwitch->SetRelativeRotation(AItemScannerEquipment::GetModeSelectorRotation(EItemScannerMode::ItemSearch));
    for (int32 I=0;I<Equipment->StatusLights.Num();++I)
    {
        auto* MID=Equipment->StatusLights[I]->CreateDynamicMaterialInstance(0);
        if (MID) MID->SetVectorParameterValue(TEXT("LEDColor"),I==1?FLinearColor(0.8f,0.025f,0.01f):FLinearColor(0.025f,0.8f,0.12f));
    }
    Equipment->ProductScreen=NewObject<UItemScannerEquipmentScreen>(Equipment);
    Equipment->ProductScreen->Initialize(); Equipment->ProductScreen->Configure(Equipment,true);
    Equipment->ProductDisplay->SetWidget(Equipment->ProductScreen);
    Equipment->ProductScreen->TakeWidget(); Equipment->ProductScreen->Refresh(false);
    TestEqual(TEXT("Selected product shown in lower window"),Equipment->ProductScreen->ProductLines[2]->GetText().ToString(),Equipment->GetProductName(0).ToString());
    TestEqual(TEXT("Product animation uses only five recycled lines"),Equipment->ProductScreen->ProductLines.Num(),5);
    Equipment->ProductScreen->AnimateProductStep(1);
    TestTrue(TEXT("Forward scrolling begins one row below"),Equipment->ProductScreen->ProductScrollOffset>0);
    Equipment->ProductScreen->AdvanceProductAnimation(0.06f);
    TestTrue(TEXT("Scroll passes through intermediate positions"),Equipment->ProductScreen->ProductScrollOffset>0 && Equipment->ProductScreen->ProductScrollOffset<68);
    Equipment->ProductScreen->AnimateProductStep(-1);
    TestTrue(TEXT("Reversal changes direction immediately"),Equipment->ProductScreen->ProductScrollOffset<0);
    Equipment->ProductScreen->AdvanceProductAnimation(1.f);
    TestEqual(TEXT("Scroll settles exactly on center"),Equipment->ProductScreen->ProductScrollOffset,0.f);
    Equipment->ResultPage=1; Equipment->MainScreen->Refresh(true);
    TestEqual(TEXT("Ninth result is on second page"),Equipment->MainScreen->RowMetrics.Num(),1);
    Equipment->ResultPage=0; Equipment->MainScreen->Refresh(true);
    Equipment->ProductDisplay->SetVisibility(true);
    Equipment->ProductDisplay->SetTickWhenOffscreen(true);
    Equipment->ProductDisplay->SetComponentTickEnabled(true);
    Equipment->ProductDisplay->SetRedrawTime(0.f);
    Equipment->MainDisplay->SetVisibility(true);
    Equipment->MainDisplay->SetTickWhenOffscreen(true);
    Equipment->MainDisplay->SetComponentTickEnabled(true);
    // The automation test runs several simulated ticks inside one real engine frame.
    // FApp time is therefore constant; disable only the test's real-time redraw throttle.
    Equipment->MainDisplay->SetRedrawTime(0.f);
    Equipment->MainDisplay->RequestRedraw();
    auto* Light=World->SpawnActor<ADirectionalLight>();
    Light->SetActorRotation(FRotator(-25,15,0)); Light->GetLightComponent()->SetIntensity(1.f);
    auto* CaptureActor=World->SpawnActor<ASceneCapture2D>();
    auto* Capture=CaptureActor->GetCaptureComponent2D();
    Capture->bCaptureEveryFrame=false; Capture->bCaptureOnMovement=false;
    const FVector Camera(-58,-8,6);
    CaptureActor->SetActorLocation(Camera); CaptureActor->SetActorRotation((FVector(0,0,-2)-Camera).Rotation());
    Capture->FOVAngle=43.f; Capture->CaptureSource=ESceneCaptureSource::SCS_FinalColorLDR;
    // Scene captures can disable world widget primitives independently of material rendering.
    Capture->ShowFlags.SetWidgetComponents(true);
    Capture->PostProcessSettings.bOverride_AutoExposureMinBrightness=true;
    Capture->PostProcessSettings.bOverride_AutoExposureMaxBrightness=true;
    Capture->PostProcessSettings.AutoExposureMinBrightness=1.f;
    Capture->PostProcessSettings.AutoExposureMaxBrightness=1.f;
    Capture->PostProcessSettings.bOverride_AutoExposureBias=true;
    Capture->PostProcessSettings.AutoExposureBias=-1.f;
    TStrongObjectPtr<UTextureRenderTarget2D> ModelImage(NewObject<UTextureRenderTarget2D>());
    ModelImage->InitCustomFormat(1200,900,PF_B8G8R8A8,false); ModelImage->UpdateResourceImmediate(true);
    Capture->TextureTarget=ModelImage.Get();
    AddInfo(FString::Printf(TEXT("World feature level %d; renderer %d; materials flag %d"),int32(World->GetFeatureLevel()),int32(GMaxRHIFeatureLevel),int32(Capture->ShowFlags.Materials)));
    TInlineComponentArray<UPrimitiveComponent*> Primitives(Equipment);
    TSet<UMaterial*> Materials;
    for (auto* Primitive : Primitives)
    {
        for (int32 Slot=0;Slot<Primitive->GetNumMaterials();++Slot)
        if (auto* Material=Primitive->GetMaterial(Slot))
        {
            AddInfo(Primitive->GetName()+TEXT(": ")+Material->GetPathName());
            Materials.Add(Material->GetMaterial());
        }
    }
    for (auto* Material : Materials) Material->ForceRecompileForRendering();
    FAssetCompilingManager::Get().FinishAllCompilation();
    if (GShaderCompilingManager) GShaderCompilingManager->FinishAllCompilation();
    for (int32 Frame=0;Frame<5;++Frame)
    {
        ++GFrameCounter; World->Tick(LEVELTICK_All,0.2f);
        // This isolated test world does not BeginPlay the game's game mode.
        Equipment->MainDisplay->RequestRedraw();
        Equipment->MainDisplay->TickComponent(0.2f,LEVELTICK_All,nullptr);
        Equipment->ProductDisplay->RequestRedraw();
        Equipment->ProductDisplay->TickComponent(0.2f,LEVELTICK_All,nullptr);
        FlushRenderingCommands();
    }
    TestNotNull(TEXT("Main physical screen render target"),Equipment->MainDisplay->GetRenderTarget());
    for (auto* Display : {Equipment->MainDisplay.Get()})
    {
        UTexture* Bound=nullptr;
        TestTrue(TEXT("Physical material has SlateUI texture"),Display->GetMaterialInstance()->GetTextureParameterValue(FMaterialParameterInfo(TEXT("SlateUI")),Bound));
        TestTrue(TEXT("Physical material uses its live display texture"),Bound==Display->GetRenderTarget());
    }
    FBufferArchive ScreenBytes;
    TestTrue(TEXT("Physical screen output exported"),FImageUtils::ExportRenderTarget2DAsPNG(Equipment->MainDisplay->GetRenderTarget(),ScreenBytes));
    FFileHelper::SaveArrayToFile(ScreenBytes,*(Out+TEXT("physical-main.png")));
    for (auto* Primitive : Primitives) Primitive->MarkRenderStateDirty();
    World->SendAllEndOfFrameUpdates(); FlushRenderingCommands();
    Capture->CaptureScene(); FlushRenderingCommands();
    // First scene submission flushes deferred render-target resource clears. Render a
    // subsequent frame, as gameplay does, before evaluating the live screen texture.
    Equipment->MainDisplay->RequestRedraw();
    Equipment->MainDisplay->TickComponent(0.2f,LEVELTICK_All,nullptr);
    Equipment->ProductDisplay->RequestRedraw();
    Equipment->ProductDisplay->TickComponent(0.2f,LEVELTICK_All,nullptr);
    World->SendAllEndOfFrameUpdates(); FlushRenderingCommands();
    Capture->CaptureScene(); FlushRenderingCommands();
    TArray<FColor> Pixels;
    TestTrue(TEXT("Read assembled equipment pixels"),ModelImage->GameThread_GetRenderTargetResource()->ReadPixels(Pixels));
    auto CountScreenText=[&Pixels](int32 X0,int32 Y0,int32 X1,int32 Y1)
    {
        int32 Count=0;
        for (int32 Y=Y0;Y<Y1;++Y) for (int32 X=X0;X<X1;++X)
        {
            const int32 Index=Y*1200+X;
            if (Pixels.IsValidIndex(Index) && Pixels[Index].G>110 && Pixels[Index].B>95) ++Count;
        }
        return Count;
    };
    TestTrue(TEXT("Main display text visible on 3D equipment"),CountScreenText(360,210,600,650)>500);
    FBufferArchive ModelBytes;
    TestTrue(TEXT("Assembled equipment render"),FImageUtils::ExportRenderTarget2DAsPNG(ModelImage.Get(),ModelBytes));
    TestTrue(TEXT("Assembled equipment preview saved"),FFileHelper::SaveArrayToFile(ModelBytes,*(Out+TEXT("equipment.png"))));
    Capture->CaptureSource=ESceneCaptureSource::SCS_BaseColor;
    Capture->CaptureScene(); FlushRenderingCommands();
    TArray<FColor> BasePixels;
    ModelImage->GameThread_GetRenderTargetResource()->ReadPixels(BasePixels);
    int32 OrangeFrontPixels=0;
    for (int32 Y=0;Y<900;++Y) for (int32 X=0;X<1200;++X)
    {
        const int32 Index=Y*1200+X;
        if (!BasePixels.IsValidIndex(Index)) continue;
        const FColor& C=BasePixels[Index];
        if (C.R>80 && C.R>C.G*1.5f && C.G>C.B*2.f) ++OrangeFrontPixels;
    }
    TestTrue(TEXT("Painted CAD front faces visible, not reversed rear casing"),OrangeFrontPixels>10000);
    FBufferArchive BaseColorBytes; FImageUtils::ExportRenderTarget2DAsPNG(ModelImage.Get(),BaseColorBytes);
    FFileHelper::SaveArrayToFile(BaseColorBytes,*(Out+TEXT("basecolor.png")));
    Equipment->State.Mode=EItemScannerMode::ConnectionCheck;
    Equipment->ModeSwitch->SetRelativeRotation(AItemScannerEquipment::GetModeSelectorRotation(EItemScannerMode::ConnectionCheck));
    Equipment->RefreshScreens(true);
    TestEqual(TEXT("Connection header synchronized"),Equipment->MainScreen->Heading->GetText().ToString(),FString(TEXT("02 / CONNECTION CHECK")));
    TestEqual(TEXT("Connection target synchronized"),Equipment->ProductScreen->ProductLines[2]->GetText().ToString(),FString(TEXT("BELT + PIPE")));
    Capture->CaptureSource=ESceneCaptureSource::SCS_FinalColorLDR;
    for (int32 Frame=0;Frame<2;++Frame)
    {
        ++GFrameCounter;
        Equipment->MainDisplay->RequestRedraw();
        Equipment->ProductDisplay->RequestRedraw();
        Equipment->MainDisplay->TickComponent(0.2f,LEVELTICK_All,nullptr);
        Equipment->ProductDisplay->TickComponent(0.2f,LEVELTICK_All,nullptr);
        World->SendAllEndOfFrameUpdates(); FlushRenderingCommands();
        Capture->CaptureScene(); FlushRenderingCommands();
    }
    FBufferArchive ConnectionBytes; FImageUtils::ExportRenderTarget2DAsPNG(ModelImage.Get(),ConnectionBytes);
    FFileHelper::SaveArrayToFile(ConnectionBytes,*(Out+TEXT("equipment-connection.png")));
    Equipment->Destroy(); GEngine->DestroyWorldContext(World); World->DestroyWorld(false);
    return true;
}
#endif
