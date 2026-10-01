#include "ItemScannerEquipment.h"
#include "ItemScannerEquipmentScreen.h"
#include "ItemScannerRadialMenu.h"
#include "ItemScannerCatalog.h"
#include "ItemScannerManager.h"
#include "ItemScannerTrackingManager.h"
#include "ItemScannerWorldSubsystem.h"
#include "ItemScannerSettings.h"
#include "ItemScannerModConfig.h"
#include "FGCharacterPlayer.h"
#include "FGPlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputTriggers.h"
#include "InputCoreTypes.h"
#include "Net/UnrealNetwork.h"
#include "Resources/FGItemDescriptor.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UnrealType.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimMontage.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/TextRenderComponent.h"

AItemScannerEquipment::AItemScannerEquipment()
{
    PrimaryActorTick.bCanEverTick=true; PrimaryActorTick.bStartWithTickEnabled=false;
    bReplicates=true;
    State.RangeMeters=GetDefault<UItemScannerSettings>()->DefaultScanRangeMeters;
    State.ConnectionGapMeters=FMath::Clamp(GetDefault<UItemScannerSettings>()->ConnectionCandidateGapCm/100.f,0.5f,2.f);
    mEquipmentSlot=EEquipmentSlot::ES_ARMS;
    mArmAnimation=EArmEquipment::AE_Generic1Hand;
    mAttachSocket=TEXT("hand_rSocket");
    // Match the stock Object Scanner hand pose and locomotion. The custom mesh
    // lives in the same local coordinate frame and follows the same hand socket.
    static ConstructorHelpers::FObjectFinder<UAnimSequence> Idle1P(TEXT("/Game/FactoryGame/Character/Player/Animation/FirstPerson/ObjectScannerIdle_01.ObjectScannerIdle_01"));
    static ConstructorHelpers::FObjectFinder<UAnimSequence> Idle3P(TEXT("/Game/FactoryGame/Character/Player/Animation/ThirdPerson/ObjectScannerIdle_01.ObjectScannerIdle_01"));
    static ConstructorHelpers::FObjectFinder<UAnimSequence> Crouch3P(TEXT("/Game/FactoryGame/Character/Player/Animation/ThirdPerson/CrouchMedkitPose_01.CrouchMedkitPose_01"));
    static ConstructorHelpers::FObjectFinder<UAnimSequence> Slide3P(TEXT("/Game/FactoryGame/Character/Player/Animation/ThirdPerson/SlideMedkitPose_01.SlideMedkitPose_01"));
    static ConstructorHelpers::FObjectFinder<UObject> IdleAO(TEXT("/Game/FactoryGame/Character/Player/Animation/ThirdPerson/Idle_AO.Idle_AO"));
    if (auto* Property=FindFProperty<FObjectProperty>(AFGEquipment::StaticClass(),TEXT("mIdlePoseAnimation")))
        Property->SetObjectPropertyValue_InContainer(this,Idle1P.Object);
    if (auto* Property=FindFProperty<FObjectProperty>(AFGEquipment::StaticClass(),TEXT("mIdlePoseAnimation3p")))
        Property->SetObjectPropertyValue_InContainer(this,Idle3P.Object);
    if (auto* Property=FindFProperty<FObjectProperty>(AFGEquipment::StaticClass(),TEXT("mCrouchPoseAnimation3p")))
        Property->SetObjectPropertyValue_InContainer(this,Crouch3P.Object);
    if (auto* Property=FindFProperty<FObjectProperty>(AFGEquipment::StaticClass(),TEXT("mSlidePoseAnimation3p")))
        Property->SetObjectPropertyValue_InContainer(this,Slide3P.Object);
    if (auto* Property=FindFProperty<FObjectProperty>(AFGEquipment::StaticClass(),TEXT("mAttachmentIdleAO")))
        Property->SetObjectPropertyValue_InContainer(this,IdleAO.Object);
    static ConstructorHelpers::FObjectFinder<UAnimMontage> Equip1P(TEXT("/Game/FactoryGame/Character/Player/Animation/FirstPerson/ObjectScannerEquip_01_Montage.ObjectScannerEquip_01_Montage"));
    static ConstructorHelpers::FObjectFinder<UAnimMontage> Equip3P(TEXT("/Game/FactoryGame/Character/Player/Animation/ThirdPerson/ObjectScannerEquip_01_Montage.ObjectScannerEquip_01_Montage"));
    FFGWeightedEquipmentMontage EquipMontage;
    EquipMontage.Montage_1P=Equip1P.Object; EquipMontage.Montage_3P=Equip3P.Object;
    mEquipMontage.Montages.Add(EquipMontage);
    mNeedsDefaultEquipmentMappingContext=false; mDefaultEquipmentActions=0;
    ModelRoot=CreateDefaultSubobject<USceneComponent>(TEXT("ScannerRoot")); SetRootComponent(ModelRoot);
    GeometryRoot=CreateDefaultSubobject<USceneComponent>(TEXT("ScannerGeometry"));
    GeometryRoot->SetupAttachment(ModelRoot);
    GeometryRoot->SetRelativeRotation(FRotator(0.f,-90.f,0.f));
    auto MakeMesh=[this](const TCHAR* Name,const FVector& Location)
    {
        auto* Mesh=CreateDefaultSubobject<UStaticMeshComponent>(Name);
        Mesh->SetupAttachment(GeometryRoot); Mesh->SetRelativeLocation(Location);
        Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); Mesh->SetGenerateOverlapEvents(false);
        Mesh->SetCastShadow(false);
        const FString Path=FString::Printf(TEXT("/ItemScanner/Equipment/%s.%s"),Name,Name);
        ConstructorHelpers::FObjectFinder<UStaticMesh> Asset(*Path);
        Mesh->SetStaticMesh(Asset.Object); return Mesh;
    };
    Body=MakeMesh(TEXT("Scanner_Body"),FVector::ZeroVector);
    RotaryDial=MakeMesh(TEXT("Scanner_RotaryDial"),FVector(-7.0,-1.4865,-7.34135));
    ModeSwitch=MakeMesh(TEXT("Scanner_ModeSwitch"),FVector(-7.0,-2.2865,-14.04135));
    MakeMesh(TEXT("Scanner_MainDisplay"),FVector(2.3,-1.5865,-4.24135));
    auto MakeScreen=[this](const TCHAR* Name,FVector Location,FIntPoint Size,float Scale)
    {
        auto* Screen=CreateDefaultSubobject<UWidgetComponent>(Name);
        Screen->SetupAttachment(GeometryRoot); Screen->SetRelativeLocation(Location);
        Screen->SetRelativeRotation(FRotator(0,-90,0)); Screen->SetRelativeScale3D(FVector(Scale));
        Screen->SetWidgetSpace(EWidgetSpace::World); Screen->SetDrawSize(Size);
        Screen->SetBlendMode(EWidgetBlendMode::Opaque); Screen->SetBackgroundColor(FLinearColor::Black);
        Screen->SetCollisionEnabled(ECollisionEnabled::NoCollision); Screen->SetGenerateOverlapEvents(false);
        Screen->SetCastShadow(false); Screen->SetTwoSided(false); Screen->SetTickWhenOffscreen(false);
        Screen->SetManuallyRedraw(true); Screen->SetRedrawTime(0.1f);
        ConstructorHelpers::FObjectFinder<UMaterialInterface> ScreenMaterial(TEXT("/ItemScanner/Equipment/M_ScannerScreen.M_ScannerScreen"));
        // Set the template's material without creating a dynamic material inside the actor constructor.
        // UWidgetComponent creates its SlateUI MID normally on registration.
        Screen->UMeshComponent::SetMaterial(0,ScreenMaterial.Object);
        return Screen;
    };
    // One millimetre above the physical glass to avoid depth fighting.
    MainDisplay=MakeScreen(TEXT("MainDisplayUI"),FVector(2.3,-1.5965,-4.24135),FIntPoint(768,826),13.f/768.f);
    ProductDisplay=MakeScreen(TEXT("ProductDisplayUI"),FVector(2.3,-1.5965,-14.14135),FIntPoint(660,270),10.8f/660.f);
    ProductDisplay->SetRedrawTime(0.f); // Only requested during scrolling or the normal 10 Hz refresh.
    auto Label=[this](const TCHAR* Name,const TCHAR* Text,FVector Location,float Size)
    {
        auto* Component=CreateDefaultSubobject<UTextRenderComponent>(Name);
        Component->SetupAttachment(GeometryRoot);
        Component->SetRelativeLocation(Location);
        Component->SetRelativeRotation(FRotator(0,-90,0));
        Component->SetHorizontalAlignment(EHTA_Center);
        Component->SetVerticalAlignment(EVRTA_TextCenter);
        Component->SetWorldSize(Size);
        Component->SetTextRenderColor(FColor(225,232,220));
        Component->SetText(FText::FromString(Text));
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetCastShadow(false);
    };
    const TCHAR* Categories[]={TEXT("STORAGE"),TEXT("PRODUCTION"),TEXT("CONVEYOR"),TEXT("LOGISTICS")};
    ConstructorHelpers::FObjectFinder<UStaticMesh> LightMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    ConstructorHelpers::FObjectFinder<UMaterialInterface> LightMaterial(TEXT("/ItemScanner/Equipment/M_StatusLED.M_StatusLED"));
    for (int32 I=0;I<4;++I)
    {
        const float Z=1.75865f-I*1.8f;
        Label(*FString::Printf(TEXT("FilterLabel%d"),I),Categories[I],FVector(-6.47,-1.9065,Z),0.42f);
        auto* Light=CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("StatusLED%d"),I));
        Light->SetupAttachment(GeometryRoot); Light->SetStaticMesh(LightMesh.Object);
        Light->SetRelativeLocation(FVector(-8.57,-2.31,Z));
        Light->SetRelativeScale3D(FVector(0.0057,0.0018,0.0057));
        Light->SetMaterial(0,LightMaterial.Object);
        Light->SetCollisionEnabled(ECollisionEnabled::NoCollision); Light->SetCastShadow(false);
        StatusLights.Add(Light);
    }
    Label(TEXT("ModeItem"),TEXT("ITEM\nRADAR"),FVector(-5.85,-1.80,-11.15),0.42f);
    Label(TEXT("ModeGap"),TEXT("GAP\nCHECK"),FVector(-8.15,-1.80,-11.15),0.42f);
    Label(TEXT("SerialMark"),TEXT("FICSIT  /  FS-02"),FVector(2.0,-3.54,3.18),0.25f);
}

void AItemScannerEquipment::Equip(AFGCharacterPlayer* Character)
{
    Super::Equip(Character);
    SetActorHiddenInGame(false); SetEquipmentTicks(true);
    bScannerActive=true; UpdateAttachment();
    StatusLightMaterials.Reset();
    for (const auto& Light : StatusLights) StatusLightMaterials.Add(Light->CreateDynamicMaterialInstance(0));
    if (!IsLocalInstigator()) return;
    auto* Controller=Cast<APlayerController>(Character->GetController()); if (!Controller) return;
    auto* Subsystem=GetWorld()->GetSubsystem<UItemScannerWorldSubsystem>(); if (!Subsystem) return;
    Catalog=Subsystem->GetCatalog();
    // Dedicated cache per equipped device: no debug HUD / other equipment ownership collision.
    Scanner=NewObject<UItemScannerManager>(this); Tracking=NewObject<UItemScannerTrackingManager>(this);
    Tracking->Initialize(Scanner);
    if (Catalog && Catalog->EnsureBuilt(this) && !Catalog->GetItems().Contains(State.TargetItem))
    {
        State.TargetItem=Catalog->GetItems()[0];
        StateChanged();
    }
    MainScreen=CreateWidget<UItemScannerEquipmentScreen>(Controller);
    if (MainScreen)
    {
        MainScreen->Configure(this,false);
        MainDisplay->SetWidget(MainScreen);
        MainScreen->TakeWidget();
    }
    Status=FText::FromString(TEXT("READY")); ResultPage=0; VisualDialDegrees=State.DialSteps*30.f;
    ProductScreen=CreateWidget<UItemScannerEquipmentScreen>(Controller);
    if (ProductScreen) { ProductScreen->Configure(this,true); ProductDisplay->SetWidget(ProductScreen); ProductScreen->TakeWidget(); }
    bHasScannedThisEquip=false;
    InstallInput(); RefreshScreens(true);
}

void AItemScannerEquipment::UpdateAttachment()
{
    auto* Character=GetInstigatorCharacter(); if (!Character) return;
    const bool bFirstPerson=IsLocalInstigator() && GetInstigatorCameraMode()==ECameraMode::ECM_FirstPerson;
    USceneComponent* Desired=bFirstPerson?static_cast<USceneComponent*>(Character->GetMesh1P()):Character->GetMesh3P();
    if (!Desired) return;
    if (GetRootComponent()->GetAttachParent()!=Desired || bFirstPersonAttached!=bFirstPerson)
    {
        AttachToComponent(Desired,FAttachmentTransformRules::SnapToTargetNotIncludingScale,mAttachSocket);
        bFirstPersonAttached=bFirstPerson;
    }
    const auto* Settings=GetDefault<UItemScannerSettings>();
    const FVector FirstPersonOffset=ItemScannerModConfig::GetFirstPersonOffset(this,Settings->FirstPersonOffset);
    SetActorRelativeLocation(bFirstPerson?FirstPersonOffset:FVector::ZeroVector);
    const FRotator FirstPersonRotation=ItemScannerModConfig::GetFirstPersonRotation(this,Settings->FirstPersonRotation);
    SetActorRelativeRotation(bFirstPerson?FirstPersonRotation:FRotator::ZeroRotator);
    const float Scale=bFirstPerson?ItemScannerModConfig::GetFirstPersonScale(this,Settings->FirstPersonScale):Settings->EquipmentScale;
    SetActorRelativeScale3D(FVector(FMath::Clamp(Scale,0.1f,3.f)));
}

void AItemScannerEquipment::UnEquip()
{
    StopScanner(); Super::UnEquip(); SetActorHiddenInGame(true); SetEquipmentTicks(false);
}
void AItemScannerEquipment::EndPlay(const EEndPlayReason::Type Reason)
{
    StopScanner(); Super::EndPlay(Reason);
}
void AItemScannerEquipment::StopScanner()
{
    CloseRadialMenu(false); bScannerActive=false; RemoveInput();
    PendingProductSteps=0; WheelPlaybackSeconds=0.f;
    if (Tracking) Tracking->StopTracking(true);
    MainDisplay->SetWidget(nullptr);
    ProductDisplay->SetWidget(nullptr); ProductScreen=nullptr;
    MainScreen=nullptr; RadialMenu=nullptr; Tracking=nullptr; Scanner=nullptr;
}
void AItemScannerEquipment::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!bScannerActive || !IsEquipped()) return;
    UpdateAttachment();
    if (ProductScreen && ProductScreen->AdvanceProductAnimation(DeltaSeconds)) ProductDisplay->RequestRedraw();
    if (PendingProductSteps!=0 && State.Mode==EItemScannerMode::ItemSearch && CanUseInput())
    {
        WheelPlaybackSeconds+=DeltaSeconds;
        const float PlaybackInterval=FMath::GetMappedRangeValueClamped(
            FVector2D(1.f,16.f),FVector2D(0.12f,0.04f),FMath::Abs(PendingProductSteps));
        if (WheelPlaybackSeconds>=PlaybackInterval)
        {
            WheelPlaybackSeconds=FMath::Fmod(WheelPlaybackSeconds,PlaybackInterval);
            const int32 Direction=FMath::Sign(PendingProductSteps);
            PendingProductSteps-=Direction;
            StepProduct(Direction);
        }
    }
    else WheelPlaybackSeconds=0.f;
    ModeSwitch->SetRelativeRotation(FMath::RInterpConstantTo(ModeSwitch->GetRelativeRotation(),GetModeSelectorRotation(State.Mode),DeltaSeconds,180.f));
    const float Delta=FMath::FindDeltaAngleDegrees(VisualDialDegrees,State.DialSteps*30.f);
    VisualDialDegrees=FMath::UnwindDegrees(VisualDialDegrees+FMath::Clamp(Delta,-1080.f*DeltaSeconds,1080.f*DeltaSeconds));
    // CAD axle is -Y, mapped to local +Z by the import transform.
    RotaryDial->SetRelativeRotation(FRotator(0,-VisualDialDegrees,0));
    for (int32 I=0;I<StatusLightMaterials.Num();++I)
        if (StatusLightMaterials[I]) StatusLightMaterials[I]->SetVectorParameterValue(TEXT("LEDColor"),
            (State.Categories&(1<<I))?FLinearColor(0.025f,0.8f,0.12f):FLinearColor(0.8f,0.025f,0.01f));
    ScreenUpdateSeconds+=DeltaSeconds;
    if (ScreenUpdateSeconds>=0.1f && IsLocalInstigator())
    {
        ScreenUpdateSeconds=0.f;
        if (Catalog && !State.TargetItem && Catalog->EnsureBuilt(this) && !Catalog->GetItems().IsEmpty())
        { State.TargetItem=Catalog->GetItems()[0]; StateChanged(); }
        RefreshScreens(false);
    }
}

bool AItemScannerEquipment::CanUseInput() const
{
    const APlayerController* PC=InputController.Get();
    return bScannerActive && IsEquipped() && IsLocalInstigator() && IsValid(PC) && !PC->bShowMouseCursor && !PC->IsMoveInputIgnored();
}
void AItemScannerEquipment::InstallInput()
{
    RemoveInput();
    auto* Character=GetInstigatorCharacter();
    auto* PC=Character?Cast<AFGPlayerController>(Character->GetController()):nullptr;
    if (!PC || !PC->IsLocalController()) return;
    InputController=PC;
    constexpr int32 ScannerInputPriority=10000;
    ScannerInput=NewObject<UEnhancedInputComponent>(this,TEXT("ScannerEnhancedInput"));
    ScannerInput->RegisterComponent(); ScannerInput->Priority=ScannerInputPriority;
    ScannerMapping=NewObject<UInputMappingContext>(this);
    using FHandler=void(AItemScannerEquipment::*)();
    UInputAction* ShiftAction=NewObject<UInputAction>(this);
    ShiftAction->ValueType=EInputActionValueType::Boolean;
    ShiftAction->bConsumeInput=false;
    ScannerActions.Add(ShiftAction);
    ScannerMapping->MapKey(ShiftAction,EKeys::LeftShift);
    ScannerMapping->MapKey(ShiftAction,EKeys::RightShift);
    const TPair<FKey,FHandler> ShiftBindings[]={
        {EKeys::MouseScrollUp,&AItemScannerEquipment::PreviousProduct},
        {EKeys::MouseScrollDown,&AItemScannerEquipment::NextProduct}};
    for (const auto& Binding : ShiftBindings)
    {
        // A higher-priority Enhanced Input chord leaves plain 1-5 to Satisfactory
        // while its generated chord blocker consumes Shift+number before the hotbar.
        UInputAction* Action=NewObject<UInputAction>(this);
        Action->ValueType=EInputActionValueType::Boolean;
        Action->bConsumeInput=true;
        ScannerActions.Add(Action);
        FEnhancedActionKeyMapping& Mapping=ScannerMapping->MapKey(Action,Binding.Key);
        UInputTriggerChordAction* Chord=NewObject<UInputTriggerChordAction>(Action);
        Chord->ChordAction=ShiftAction;
        Mapping.Triggers.Add(Chord);
        ScannerInput->BindAction(Action,ETriggerEvent::Started,this,Binding.Value);
    }
    const TPair<FKey,FHandler> Bindings[]={
        {EKeys::RightMouseButton,&AItemScannerEquipment::Scan},
        {EKeys::R,&AItemScannerEquipment::NextPage}};
    for (const auto& Binding : Bindings)
    {
        UInputAction* Action=NewObject<UInputAction>(this); Action->ValueType=EInputActionValueType::Boolean;
        Action->bConsumeInput=true; ScannerActions.Add(Action); ScannerMapping->MapKey(Action,Binding.Key);
        ScannerInput->BindAction(Action,ETriggerEvent::Started,this,Binding.Value);
    }
    UInputAction* MenuAction=NewObject<UInputAction>(this); MenuAction->ValueType=EInputActionValueType::Boolean;
    MenuAction->bConsumeInput=true; ScannerActions.Add(MenuAction); ScannerMapping->MapKey(MenuAction,EKeys::LeftMouseButton);
    ScannerInput->BindAction(MenuAction,ETriggerEvent::Started,this,&AItemScannerEquipment::MenuPressed);
    ScannerInput->BindAction(MenuAction,ETriggerEvent::Completed,this,&AItemScannerEquipment::MenuReleased);
    UInputAction* MouseXAction=NewObject<UInputAction>(this); MouseXAction->ValueType=EInputActionValueType::Axis1D;
    MouseXAction->bConsumeInput=false; ScannerActions.Add(MouseXAction); ScannerMapping->MapKey(MouseXAction,EKeys::MouseX);
    ScannerInput->BindAction(MouseXAction,ETriggerEvent::Triggered,this,&AItemScannerEquipment::MenuAxisX);
    UInputAction* MouseYAction=NewObject<UInputAction>(this); MouseYAction->ValueType=EInputActionValueType::Axis1D;
    MouseYAction->bConsumeInput=false; ScannerActions.Add(MouseYAction); ScannerMapping->MapKey(MouseYAction,EKeys::MouseY);
    ScannerInput->BindAction(MouseYAction,ETriggerEvent::Triggered,this,&AItemScannerEquipment::MenuAxisY);
    PC->PushInputComponent(ScannerInput);
    ScannerMappingHandle=PC->BindMappingContext(ScannerMapping,ScannerInputPriority,this);
}
void AItemScannerEquipment::RemoveInput()
{
    if (auto* PC=Cast<AFGPlayerController>(InputController.Get()))
    {
        if (ScannerMapping) PC->UnbindMappingContext(ScannerMapping,ScannerMappingHandle);
        if (ScannerInput) PC->PopInputComponent(ScannerInput);
    }
    if (ScannerInput) ScannerInput->DestroyComponent();
    ScannerInput=nullptr; ScannerMapping=nullptr; ScannerActions.Reset(); InputController.Reset();
}
void AItemScannerEquipment::StepProduct(int32 Step)
{
    if (!CanUseInput() || State.Mode!=EItemScannerMode::ItemSearch || !Catalog || !Catalog->EnsureBuilt(this)) return;
    const auto& Items=Catalog->GetItems();
    const int32 Current=FMath::Max(0,Items.IndexOfByKey(State.TargetItem));
    State.TargetItem=Items[UItemScannerCatalog::WrapIndex(Current+Step,Items.Num())];
    State.DialSteps=UItemScannerCatalog::WrapIndex(State.DialSteps+Step,12);
    StateChanged();
    if (ProductScreen)
    {
        const float Duration=FMath::GetMappedRangeValueClamped(FVector2D(1.f,16.f),FVector2D(0.12f,0.04f),FMath::Abs(PendingProductSteps));
        ProductScreen->AnimateProductStep(Step,Duration); ProductDisplay->RequestRedraw();
    }
}
int32 AItemScannerEquipment::GetAcceleratedWheelStep(double IntervalSeconds, int32 BurstCount)
{
    if (IntervalSeconds>0.35 || BurstCount<=1) return 1;
    if (BurstCount==2) return 2;
    if (BurstCount==3) return 3;
    if (BurstCount<=5) return 5;
    if (BurstCount>=6) return 8;
    return 1;
}
void AItemScannerEquipment::WheelProduct(int32 Direction)
{
    if (!CanUseInput() || State.Mode!=EItemScannerMode::ItemSearch) return;
    const double Now=FPlatformTime::Seconds();
    const double Interval=Direction==LastWheelDirection && LastWheelSeconds>=0.0
        ? Now-LastWheelSeconds
        : TNumericLimits<double>::Max();
    WheelBurstCount=Interval<=0.35?WheelBurstCount+1:1;
    LastWheelSeconds=Now;
    if (LastWheelDirection!=0 && LastWheelDirection!=Direction) PendingProductSteps=0;
    LastWheelDirection=Direction;
    PendingProductSteps=FMath::Clamp(PendingProductSteps+Direction*GetAcceleratedWheelStep(Interval,WheelBurstCount),-16,16);
}
void AItemScannerEquipment::MenuPressed()
{
    if (CanUseInput()) OpenRadialMenu();
}
void AItemScannerEquipment::MenuReleased()
{
    CloseRadialMenu(true);
}
void AItemScannerEquipment::MenuAxisX(const FInputActionValue& Value)
{
    if (!bRadialMenuOpen) return;
    RadialDirection.X+=Value.Get<float>()*18.f;
    UpdateRadialSelection();
}
void AItemScannerEquipment::MenuAxisY(const FInputActionValue& Value)
{
    if (!bRadialMenuOpen) return;
    RadialDirection.Y-=Value.Get<float>()*18.f;
    UpdateRadialSelection();
}
void AItemScannerEquipment::OpenRadialMenu()
{
    if (bRadialMenuOpen) return;
    auto* PC=Cast<APlayerController>(InputController.Get()); if (!PC) return;
    PendingProductSteps=0; WheelPlaybackSeconds=0.f;
    RadialDirection=FVector2D::ZeroVector; RadialSelection=INDEX_NONE; bRadialMenuOpen=true;
    RadialMenu=CreateWidget<UItemScannerRadialMenu>(PC);
    if (RadialMenu) { RadialMenu->Configure(this); RadialMenu->AddToViewport(9000); }
    PC->SetIgnoreLookInput(true);
}
void AItemScannerEquipment::CloseRadialMenu(bool bCommitSelection)
{
    if (!bRadialMenuOpen) return;
    const int32 Selection=RadialSelection;
    bRadialMenuOpen=false; RadialSelection=INDEX_NONE; RadialDirection=FVector2D::ZeroVector;
    if (RadialMenu) RadialMenu->RemoveFromParent();
    RadialMenu=nullptr;
    if (auto* PC=Cast<APlayerController>(InputController.Get())) PC->SetIgnoreLookInput(false);
    if (bCommitSelection && Selection>=0 && Selection<4) ToggleCategory(Selection);
    else if (bCommitSelection && Selection==4) ToggleMode();
    else if (bCommitSelection && Selection==5) CycleConnectionGap();
    else if (bCommitSelection && Selection==6) CycleScanRange();
}
void AItemScannerEquipment::UpdateRadialSelection()
{
    const double Length=RadialDirection.Size();
    if (Length>240.0) RadialDirection=RadialDirection.GetSafeNormal()*240.0;
    int32 Best=INDEX_NONE;
    if (Length>=35.0)
    {
        const FVector2D Normal=RadialDirection.GetSafeNormal();
        double BestDot=-2.0;
        for (int32 I=0;I<7;++I)
        {
            const float Radians=FMath::DegreesToRadians(I*(360.f/7.f));
            const FVector2D Direction(FMath::Sin(Radians),-FMath::Cos(Radians));
            const double Dot=FVector2D::DotProduct(Normal,Direction);
            if (Dot>BestDot) { BestDot=Dot; Best=I; }
        }
    }
    RadialSelection=Best;
    if (RadialMenu) RadialMenu->SetSelection(Best);
}
FText AItemScannerEquipment::GetProductName(int32 Offset) const
{
    if (Offset==0 && State.TargetItem) return UFGItemDescriptor::GetItemName(State.TargetItem);
    if (!Catalog || Catalog->GetItems().IsEmpty()) return FText::FromString(TEXT("Loading products..."));
    const auto& Items=Catalog->GetItems();
    const int32 I=UItemScannerCatalog::WrapIndex(FMath::Max(0,Items.IndexOfByKey(State.TargetItem))+Offset,Items.Num());
    return UFGItemDescriptor::GetItemName(Items[I]);
}
void AItemScannerEquipment::ToggleCategory(int32 Index)
{
    if (!CanUseInput() || State.Mode!=EItemScannerMode::ItemSearch) return;
    State.Categories^=1<<Index; StateChanged();
}
FRotator AItemScannerEquipment::GetModeSelectorRotation(EItemScannerMode Mode)
{
    // CAD index points from the axle (-70.297,101.2605) towards
    // (-81.0177,92.2568), expressed in imported X/Z coordinates.
    const float IndexAngle=FMath::Atan2(-1.07207f,0.90037f);
    const float LabelAngle=FMath::Atan2(Mode==EItemScannerMode::ItemSearch?1.15f:-1.15f,2.89135f);
    return FRotator(FMath::RadiansToDegrees(IndexAngle-LabelAngle),0,0);
}
void AItemScannerEquipment::ToggleMode()
{
    if (!CanUseInput()) return;
    PendingProductSteps=0;
    State.Mode=State.Mode==EItemScannerMode::ItemSearch?EItemScannerMode::ConnectionCheck:EItemScannerMode::ItemSearch;
    StateChanged();
}
float AItemScannerEquipment::GetNextConnectionGapMeters(float Current)
{
    if (FMath::IsNearlyEqual(Current,0.5f)) return 1.f;
    if (FMath::IsNearlyEqual(Current,1.f)) return 2.f;
    return 0.5f;
}
float AItemScannerEquipment::GetNextScanRangeMeters(float Current)
{
    if (FMath::IsNearlyEqual(Current,100.f)) return 300.f;
    if (FMath::IsNearlyEqual(Current,300.f)) return 500.f;
    return 100.f;
}
void AItemScannerEquipment::CycleConnectionGap()
{
    if (!CanUseInput()) return;
    State.ConnectionGapMeters=GetNextConnectionGapMeters(State.ConnectionGapMeters);
    StateChanged();
}
void AItemScannerEquipment::CycleScanRange()
{
    if (!CanUseInput()) return;
    State.RangeMeters=GetNextScanRangeMeters(State.RangeMeters);
    StateChanged();
}
void AItemScannerEquipment::NextPage()
{
    if (!CanUseInput() || !Scanner) return;
    ResultPage=(ResultPage+1)%FMath::Max(1,FMath::DivideAndRoundUp(Scanner->GetCachedResults().Num(),ResultsPerPage));
    RefreshScreens(true);
}
void AItemScannerEquipment::Scan()
{
    if (!CanUseInput() || !Scanner || !Tracking) return;
    PendingProductSteps=0; WheelPlaybackSeconds=0.f;
    bHasScannedThisEquip=true;
    Tracking->StopTracking(false);
    FItemScannerScanRequest Request; Request.Mode=State.Mode; Request.TargetItem=State.TargetItem;
    Request.RangeMeters=State.RangeMeters; Request.ConnectionGapMeters=State.ConnectionGapMeters;
    Request.Categories.bStorage=(State.Categories&1)!=0; Request.Categories.bProduction=(State.Categories&2)!=0;
    Request.Categories.bConveyor=(State.Categories&4)!=0; Request.Categories.bLogistics=(State.Categories&8)!=0;
    FText Error;
    if (Scanner->ExecuteScan(InputController.Get(),Request,Error))
    {
        Tracking->StartTracking(InputController.Get());
        Status=FText::FromString(State.Mode==EItemScannerMode::ConnectionCheck?TEXT("CANDIDATES"):TEXT("TRACKING"));
    }
    else Status=Error;
    ResultPage=0; RefreshScreens(true);
}
void AItemScannerEquipment::StateChanged()
{
    if (HasAuthority()) FlushItemState(); else ServerSetScannerState(State);
    RefreshScreens(false);
}
void AItemScannerEquipment::ServerSetScannerState_Implementation(FItemScannerEquipmentState NewState)
{
    if (!IsEquipped()) return;
    NewState.Categories&=15;
    if (NewState.Mode!=EItemScannerMode::ItemSearch && NewState.Mode!=EItemScannerMode::ConnectionCheck) return;
    NewState.RangeMeters=FMath::IsFinite(NewState.RangeMeters)?FMath::Clamp(NewState.RangeMeters,1.f,GetDefault<UItemScannerSettings>()->MaxScanRangeMeters):500.f;
    NewState.ConnectionGapMeters=FMath::IsFinite(NewState.ConnectionGapMeters)
        ? FMath::Clamp(NewState.ConnectionGapMeters,0.5f,2.f) : 1.f;
    NewState.DialSteps=UItemScannerCatalog::WrapIndex(NewState.DialSteps,12);
    State=NewState; FlushItemState();
}
void AItemScannerEquipment::OnRep_State() { RefreshScreens(false); }
void AItemScannerEquipment::RefreshScreens(bool bRebuildResults)
{
    if (MainScreen) { MainScreen->Refresh(bRebuildResults); MainDisplay->RequestRedraw(); }
    if (ProductScreen) { ProductScreen->Refresh(false); ProductDisplay->RequestRedraw(); }
}
void AItemScannerEquipment::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps); DOREPLIFETIME(AItemScannerEquipment,State);
}
void AItemScannerEquipment::LoadFromItemState_Implementation(const FFGDynamicStruct& ItemState)
{
    Super::LoadFromItemState_Implementation(ItemState);
    if (const auto* Saved=ItemState.GetValuePtr<FItemScannerEquipmentState>()) State=*Saved;
    State.Categories&=15; State.DialSteps=UItemScannerCatalog::WrapIndex(State.DialSteps,12);
    if (!FMath::IsFinite(State.RangeMeters) || State.RangeMeters<=0) State.RangeMeters=GetDefault<UItemScannerSettings>()->DefaultScanRangeMeters;
    if (!FMath::IsFinite(State.ConnectionGapMeters) || State.ConnectionGapMeters<0.5f || State.ConnectionGapMeters>2.f)
        State.ConnectionGapMeters=1.f;
    RefreshScreens(true);
}
FFGDynamicStruct AItemScannerEquipment::SaveToItemState_Implementation() const { return FFGDynamicStruct(State); }
