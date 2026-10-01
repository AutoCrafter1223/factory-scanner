#include "ItemScannerRadialMenu.h"
#include "ItemScannerEquipment.h"
#include "ItemScannerRadialDialWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"

namespace
{
    const FLinearColor MenuShade(0.005f,0.008f,0.01f,0.78f);
    const FLinearColor Active(0.12f,0.48f,0.52f,0.98f);
    const FLinearColor Selected(0.95f,0.42f,0.055f,1.f);
}

void UItemScannerRadialMenu::Configure(AItemScannerEquipment* Equipment)
{
    ScannerEquipment=Equipment;
    RefreshVisuals();
}

TSharedRef<SWidget> UItemScannerRadialMenu::RebuildWidget()
{
    if (!WidgetTree) WidgetTree=NewObject<UWidgetTree>(this,TEXT("ScannerRadialTree"));
    if (!WidgetTree->RootWidget)
    {
        auto* Root=WidgetTree->ConstructWidget<UCanvasPanel>();
        WidgetTree->RootWidget=Root;
        auto* Shade=WidgetTree->ConstructWidget<UBorder>();
        Shade->SetBrushColor(MenuShade);
        auto* ShadeSlot=Root->AddChildToCanvas(Shade);
        ShadeSlot->SetAnchors(FAnchors(0,0,1,1)); ShadeSlot->SetOffsets(FMargin(0));

        Dial=WidgetTree->ConstructWidget<UItemScannerRadialDialWidget>();
        auto* DialSlot=Root->AddChildToCanvas(Dial);
        DialSlot->SetAnchors(FAnchors(0.5f)); DialSlot->SetAlignment(FVector2D(0.5f));
        DialSlot->SetPosition(FVector2D::ZeroVector); DialSlot->SetSize(FVector2D(760.f));

        CenterLabel=WidgetTree->ConstructWidget<UTextBlock>();
        CenterLabel->SetText(FText::FromString(TEXT("MOVE MOUSE\nRELEASE TO SELECT")));
        CenterLabel->SetJustification(ETextJustify::Center);
        auto CenterFont=CenterLabel->GetFont(); CenterFont.Size=24; CenterFont.TypefaceFontName=TEXT("Bold"); CenterLabel->SetFont(CenterFont);
        CenterLabel->SetColorAndOpacity(FLinearColor::White);
        auto* CenterSlot=Root->AddChildToCanvas(CenterLabel);
        CenterSlot->SetAnchors(FAnchors(0.5f)); CenterSlot->SetAlignment(FVector2D(0.5f));
        CenterSlot->SetPosition(FVector2D::ZeroVector); CenterSlot->SetSize(FVector2D(300,86));

        constexpr float LabelRadius=245.f;
        for (int32 I=0;I<7;++I)
        {
            const float Radians=FMath::DegreesToRadians(I*(360.f/7.f));
            const FVector2D Position(FMath::Sin(Radians)*LabelRadius,-FMath::Cos(Radians)*LabelRadius);
            auto* Label=WidgetTree->ConstructWidget<UTextBlock>(); Label->SetJustification(ETextJustify::Center);
            auto Font=Label->GetFont(); Font.Size=17; Font.TypefaceFontName=TEXT("Bold"); Label->SetFont(Font);
            Label->SetColorAndOpacity(FLinearColor::White);
            auto* CanvasSlot=Root->AddChildToCanvas(Label); CanvasSlot->SetAnchors(FAnchors(0.5f)); CanvasSlot->SetAlignment(FVector2D(0.5f));
            CanvasSlot->SetPosition(Position); CanvasSlot->SetSize(FVector2D(180,72));
            OptionLabels.Add(Label);
        }
    }
    RefreshVisuals();
    return Super::RebuildWidget();
}

void UItemScannerRadialMenu::SetSelection(int32 Index)
{
    if (SelectedIndex==Index) return;
    SelectedIndex=Index;
    RefreshVisuals();
}

void UItemScannerRadialMenu::RefreshVisuals()
{
    auto* Equipment=ScannerEquipment.Get();
    if (!Equipment || OptionLabels.Num()!=7) return;
    const auto& State=Equipment->GetScannerState();
    static const TCHAR* Names[]={TEXT("1  STORAGE"),TEXT("2  PRODUCTION"),TEXT("3  CONVEYOR"),TEXT("4  LOGISTICS")};
    for (int32 I=0;I<4;++I)
    {
        const bool bOn=(State.Categories&(1<<I))!=0;
        OptionLabels[I]->SetText(FText::FromString(FString::Printf(TEXT("%s\n%s"),Names[I],bOn?TEXT("ON"):TEXT("OFF"))));
        OptionLabels[I]->SetColorAndOpacity(I==SelectedIndex?Selected:bOn?FLinearColor::White:Active);
    }
    OptionLabels[4]->SetText(FText::FromString(State.Mode==EItemScannerMode::ItemSearch?TEXT("5  MODE\nITEM SEARCH"):TEXT("5  MODE\nCONNECTION CHECK")));
    OptionLabels[4]->SetColorAndOpacity(SelectedIndex==4?Selected:FLinearColor::White);
    OptionLabels[5]->SetText(FText::FromString(FString::Printf(TEXT("GAP DISTANCE\n%.1fm"),State.ConnectionGapMeters)));
    OptionLabels[5]->SetColorAndOpacity(SelectedIndex==5?Selected:FLinearColor::White);
    OptionLabels[6]->SetText(FText::FromString(FString::Printf(TEXT("RADAR RANGE\n%.0fm"),State.RangeMeters)));
    OptionLabels[6]->SetColorAndOpacity(SelectedIndex==6?Selected:FLinearColor::White);
    if (Dial) Dial->SetSelection(SelectedIndex);
    if (CenterLabel) CenterLabel->SetText(FText::FromString(SelectedIndex>=0?TEXT("RELEASE TO SELECT"):TEXT("MOVE MOUSE\nRELEASE TO CLOSE")));
}
