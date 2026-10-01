#include "ItemScannerEquipmentScreen.h"
#include "ItemScannerEquipment.h"
#include "ItemScannerManager.h"
#include "ItemScannerDirectionWidget.h"
#include "ItemScannerLocalization.h"
#include "Resources/FGItemDescriptor.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

namespace {
const FLinearColor Cyan(0.2f,0.83f,0.88f), White(0.76f,0.8f,0.78f), Dim(0.29f,0.37f,0.39f);
const FLinearColor Amber(1.f,0.36f,0.055f), Ink(0.0015f,0.003f,0.004f);
}
void UItemScannerEquipmentScreen::Configure(AItemScannerEquipment* Equipment, bool bProduct)
{
    ScannerEquipment=Equipment; bProductScreen=bProduct;
}
UTextBlock* UItemScannerEquipmentScreen::AddText(UVerticalBox* Parent,const FString& Text,int32 Size,const FLinearColor& Color)
{
    auto* Label=WidgetTree->ConstructWidget<UTextBlock>();
    FSlateFontInfo Font=Label->GetFont(); Font.Size=Size; Font.TypefaceFontName=TEXT("Regular"); Label->SetFont(Font);
    Label->SetColorAndOpacity(Color); Label->SetText(FText::FromString(Text));
    Label->SetAutoWrapText(true);
    Parent->AddChildToVerticalBox(Label)->SetPadding(FMargin(0,2));
    return Label;
}
TSharedRef<SWidget> UItemScannerEquipmentScreen::RebuildWidget()
{
    if (!WidgetTree) WidgetTree=NewObject<UWidgetTree>(this,TEXT("EquipmentScreenTree"));
    if (!WidgetTree->RootWidget)
    {
        auto* Background=WidgetTree->ConstructWidget<UBorder>();
        Background->SetBrushColor(Ink);
        Background->SetPadding(FMargin(18));
        auto* Layout=WidgetTree->ConstructWidget<UVerticalBox>(); Background->SetContent(Layout);
        WidgetTree->RootWidget=Background;
        if (bProductScreen)
        {
            Background->SetPadding(FMargin(16,8));
            Heading=AddText(Layout,ItemScannerLocalization::String(TEXT("SELECTED PRODUCT"),TEXT("선택 제품")),18,Amber);
            auto* Viewport=WidgetTree->ConstructWidget<UCanvasPanel>();
            Viewport->SetClipping(EWidgetClipping::ClipToBounds);
            Layout->AddChildToVerticalBox(Viewport)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
            auto* Highlight=WidgetTree->ConstructWidget<UBorder>();
            Highlight->SetBrushColor(FLinearColor(0.065f,0.035f,0.012f));
            auto* HighlightSlot=Viewport->AddChildToCanvas(Highlight);
            HighlightSlot->SetPosition(FVector2D(0,68)); HighlightSlot->SetSize(FVector2D(628,68));
            for (int32 I=0;I<5;++I)
            {
                auto* Line=WidgetTree->ConstructWidget<UTextBlock>();
                auto Font=Line->GetFont(); Font.Size=I==2?36:28; Line->SetFont(Font);
                Line->SetColorAndOpacity(I==2?Amber:White);
                Line->SetAutoWrapText(false); Line->SetTextOverflowPolicy(ETextOverflowPolicy::Ellipsis);
                auto* LineSlot=Viewport->AddChildToCanvas(Line);
                LineSlot->SetPosition(FVector2D(10,(I-1)*68+12)); LineSlot->SetSize(FVector2D(608,52));
                ProductLines.Add(Line);
            }
            Footer=AddText(Layout,ItemScannerLocalization::String(TEXT("SHIFT + WHEEL"),TEXT("SHIFT + 휠")),14,Dim);
            return Super::RebuildWidget();
        }
        auto* Header=WidgetTree->ConstructWidget<UBorder>(); Header->SetBrushColor(Amber); Header->SetPadding(FMargin(12,7));
        auto* HeaderLayout=WidgetTree->ConstructWidget<UVerticalBox>(); Header->SetContent(HeaderLayout);
        Layout->AddChildToVerticalBox(Header)->SetPadding(FMargin(0,0,0,12));
        Heading=AddText(HeaderLayout,ItemScannerLocalization::String(TEXT("FACTORY SCANNER"),TEXT("팩토리 스캐너")),bProductScreen?22:27,Ink);
        auto HeaderFont=Heading->GetFont(); HeaderFont.TypefaceFontName=TEXT("Bold"); Heading->SetFont(HeaderFont);
        if (bProductScreen)
        {
            for (int32 I=0;I<3;++I)
            {
                auto* Selection=WidgetTree->ConstructWidget<UBorder>();
                Selection->SetBrushColor(I==1?FLinearColor(0.065f,0.035f,0.012f):Ink);
                Selection->SetPadding(FMargin(12,4)); Selection->SetVerticalAlignment(VAlign_Center);
                auto* LineLayout=WidgetTree->ConstructWidget<UVerticalBox>(); Selection->SetContent(LineLayout);
                auto* Line=AddText(LineLayout,TEXT("—"),I==1?34:26,I==1?Amber:Dim);
                ProductLines.Add(Line);
                auto* SelectionSlot=Layout->AddChildToVerticalBox(Selection); SelectionSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
                SelectionSlot->SetPadding(FMargin(0,3));
            }
            AddText(Layout,ItemScannerLocalization::String(TEXT("ROTARY  /  SHIFT + WHEEL"),TEXT("로터리  /  SHIFT + 휠")),17,Dim);
        }
        else
        {
            Subheading=AddText(Layout,TEXT(""),21,Dim);
            Results=WidgetTree->ConstructWidget<UVerticalBox>();
            Results->SetClipping(EWidgetClipping::ClipToBounds);
            Layout->AddChildToVerticalBox(Results)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
            Footer=AddText(Layout,ItemScannerLocalization::String(TEXT("RMB SCAN | LMB MENU | R NEXT PAGE"),TEXT("우클릭 스캔 | 좌클릭 메뉴 | R 다음 페이지")),18,Dim);
        }
    }
    return Super::RebuildWidget();
}
void UItemScannerEquipmentScreen::Refresh(bool bRebuildResults)
{
    auto* Equipment=ScannerEquipment.Get();
    if (!Equipment || !Heading) return;
    const auto& State=Equipment->GetScannerState();
    const bool bConnections=State.Mode==EItemScannerMode::ConnectionCheck;
    const bool bKorean=ItemScannerLocalization::IsKorean();
    if (bProductScreen)
    {
        if (ProductLines.Num()==5)
        {
            Heading->SetText(bConnections
                ? ItemScannerLocalization::Text(TEXT("CONNECTION CHECK"),TEXT("연결 확인"))
                : ItemScannerLocalization::Text(TEXT("SELECTED PRODUCT"),TEXT("선택 제품")));
            Footer->SetText(bConnections
                ? ItemScannerLocalization::Text(TEXT("RMB TO SCAN"),TEXT("우클릭으로 스캔"))
                : ItemScannerLocalization::Text(TEXT("SHIFT + WHEEL"),TEXT("SHIFT + 휠")));
            for (int32 I=0;I<5;++I)
                ProductLines[I]->SetText(bConnections
                    ? (I==2?ItemScannerLocalization::Text(TEXT("BELT + PIPE"),TEXT("벨트 + 파이프")):FText::GetEmpty())
                    : Equipment->GetProductName(I-2));
            if (bConnections) { ProductScrollOffset=0; AdvanceProductAnimation(0); }
            return;
        }
        if (ProductLines.Num()==1)
        {
            Heading->SetText(bConnections
                ? ItemScannerLocalization::Text(TEXT("CONNECTION CHECK"),TEXT("연결 확인"))
                : ItemScannerLocalization::Text(TEXT("SELECTED PRODUCT"),TEXT("선택 제품")));
            ProductLines[0]->SetText(bConnections?ItemScannerLocalization::Text(TEXT("BELT + PIPE"),TEXT("벨트 + 파이프")):Equipment->GetProductName(0));
            Footer->SetText(bConnections
                ? ItemScannerLocalization::Text(TEXT("RMB TO SCAN"),TEXT("우클릭으로 스캔"))
                : ItemScannerLocalization::Text(TEXT("SHIFT + WHEEL"),TEXT("SHIFT + 휠")));
            return;
        }
        Heading->SetText(bConnections
            ? ItemScannerLocalization::Text(TEXT("DIAGNOSTIC MODE"),TEXT("진단 모드"))
            : ItemScannerLocalization::Text(TEXT("TARGET PRODUCT"),TEXT("대상 제품")));
        if (ProductLines.Num()==3)
        {
            for (int32 I=0;I<3;++I)
            {
                FString Text=bConnections
                    ? (I==0?ItemScannerLocalization::String(TEXT("CONNECTION CHECK"),TEXT("연결 확인"))
                        :I==1?ItemScannerLocalization::String(TEXT("BELT + PIPE"),TEXT("벨트 + 파이프"))
                        :ItemScannerLocalization::String(TEXT("RMB TO SCAN"),TEXT("우클릭으로 스캔")))
                    :Equipment->GetProductName(I-1).ToString();
                if (!bConnections && I==1) Text=TEXT("> ")+Text+TEXT(" <");
                ProductLines[I]->SetText(FText::FromString(Text));
            }
        }
        return;
    }
    auto* Manager=Equipment->GetScanner(); if (!Manager) return;
    const auto& Matches=Manager->GetCachedResults();
    Heading->SetText(bConnections
        ? ItemScannerLocalization::Text(TEXT("02 / CONNECTION CHECK"),TEXT("02 / 연결 확인"))
        : ItemScannerLocalization::Text(TEXT("01 / ITEM SEARCH"),TEXT("01 / 제품 검색")));
    const auto& Last=Manager->GetLastRequest();
    const FString ScanTarget=Last.Mode==EItemScannerMode::ConnectionCheck
        ? ItemScannerLocalization::String(TEXT("BELT + PIPE"),TEXT("벨트 + 파이프"))
        : Last.TargetItem?UFGItemDescriptor::GetItemName(Last.TargetItem).ToString():TEXT("—");
    const TCHAR* On=ItemScannerLocalization::Choose(TEXT("ON"),TEXT("켬"));
    const TCHAR* Off=ItemScannerLocalization::Choose(TEXT("OFF"),TEXT("끔"));
    const FString CategoryLine=FString::Printf(TEXT("1 %s  2 %s  3 %s  4 %s"),(State.Categories&1)?On:Off,(State.Categories&2)?On:Off,
        (State.Categories&4)?On:Off,(State.Categories&8)?On:Off);
    const FString RangeLine=bConnections
        ? (bKorean
            ? FString::Printf(TEXT("거리 %.0fm  |  틈새 %.1fm  |  %s"),State.RangeMeters,State.ConnectionGapMeters,*Equipment->GetStatus().ToString())
            : FString::Printf(TEXT("RANGE %.0fm  |  GAP %.1fm  |  %s"),State.RangeMeters,State.ConnectionGapMeters,*Equipment->GetStatus().ToString()))
        : (bKorean
            ? FString::Printf(TEXT("거리 %.0fm  |  %s"),State.RangeMeters,*Equipment->GetStatus().ToString())
            : FString::Printf(TEXT("RANGE %.0fm  |  %s"),State.RangeMeters,*Equipment->GetStatus().ToString()));
    const FString SubheadingText=Equipment->ShouldShowInstructions()
        ? (bKorean
            ? FString::Printf(TEXT("%s\n%s\n스캔: %s"),*RangeLine,*CategoryLine,*ScanTarget)
            : FString::Printf(TEXT("%s\n%s\nSCAN: %s"),*RangeLine,*CategoryLine,*ScanTarget))
        : (bKorean
            ? FString::Printf(TEXT("%s\n스캔: %s"),*RangeLine,*ScanTarget)
            : FString::Printf(TEXT("%s\nSCAN: %s"),*RangeLine,*ScanTarget));
    Subheading->SetText(FText::FromString(SubheadingText));
    const int32 PageSize=AItemScannerEquipment::ResultsPerPage;
    const int32 Start=Equipment->GetResultPage()*PageSize;
    if (bRebuildResults)
    {
        Results->ClearChildren(); RowMetrics.Reset(); RowArrows.Reset();
        UVerticalBox* Columns[2]={nullptr,nullptr};
        if (!Matches.IsEmpty())
        {
            auto* Grid=WidgetTree->ConstructWidget<UHorizontalBox>();
            Results->AddChildToVerticalBox(Grid);
            for (int32 Column=0;Column<2;++Column)
            {
                Columns[Column]=WidgetTree->ConstructWidget<UVerticalBox>();
                auto* ColumnSlot=Grid->AddChildToHorizontalBox(Columns[Column]);
                ColumnSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
                ColumnSlot->SetPadding(FMargin(Column==0?0:6,0,Column==0?6:0,0));
            }
        }
        for (int32 I=Start;I<FMath::Min(Start+PageSize,Matches.Num());++I)
        {
            const auto& R=Matches[I];
            auto* CardBackground=WidgetTree->ConstructWidget<UBorder>();
            CardBackground->SetBrushColor(FLinearColor(0.005f,0.009f,0.011f)); CardBackground->SetPadding(FMargin(10,3));
            auto* Card=WidgetTree->ConstructWidget<UVerticalBox>(); CardBackground->SetContent(Card);
            Columns[(I-Start)/4]->AddChildToVerticalBox(CardBackground)->SetPadding(FMargin(0,3));
            auto* Name=AddText(Card,FString::Printf(TEXT("%02d  %s"),I+1,*R.ObjectDisplayName.ToString()),25,White);
            Name->SetAutoWrapText(false); Name->SetTextOverflowPolicy(ETextOverflowPolicy::Ellipsis);
            auto* Row=WidgetTree->ConstructWidget<UHorizontalBox>(); Card->AddChildToVerticalBox(Row);
            auto* Box=WidgetTree->ConstructWidget<USizeBox>(); Box->SetWidthOverride(32); Box->SetHeightOverride(32);
            auto* Arrow=WidgetTree->ConstructWidget<UItemScannerDirectionWidget>(); Box->SetContent(Arrow);
            Row->AddChildToHorizontalBox(Box)->SetPadding(FMargin(0,0,5,0)); RowArrows.Add(Arrow);
            auto* Metric=WidgetTree->ConstructWidget<UTextBlock>();
            auto Font=Metric->GetFont(); Font.Size=27; Metric->SetFont(Font); Metric->SetColorAndOpacity(Cyan);
            Row->AddChildToHorizontalBox(Metric); RowMetrics.Add(Metric);
            const TCHAR* Categories[]={
                ItemScannerLocalization::Choose(TEXT("STORAGE"),TEXT("저장")),
                ItemScannerLocalization::Choose(TEXT("PRODUCTION"),TEXT("생산")),
                ItemScannerLocalization::Choose(TEXT("CONVEYOR"),TEXT("컨베이어")),
                ItemScannerLocalization::Choose(TEXT("LOGISTICS"),TEXT("물류"))};
            const FString DetailText=R.bConnectionCandidate
                ? (bKorean
                    ? FString::Printf(TEXT("%s | 틈새 %.1fcm"),R.bPipeline?TEXT("파이프"):TEXT("벨트"),R.GapCentimeters)
                    : FString::Printf(TEXT("%s | GAP %.1fcm"),R.bPipeline?TEXT("PIPE"):TEXT("BELT"),R.GapCentimeters))
                : FString::Printf(TEXT("%s | x%lld"),Categories[static_cast<uint8>(R.Category)],R.ItemAmount);
            auto* Detail=AddText(Card,DetailText,18,Dim);
            Detail->SetAutoWrapText(false); Detail->SetTextOverflowPolicy(ETextOverflowPolicy::Ellipsis);
        }
        if (Matches.IsEmpty()) AddText(Results,Equipment->ShouldShowInstructions()
            ? ItemScannerLocalization::String(
                TEXT("CONTROLS\nHOLD LMB  CONTROL MENU\nRMB  SCAN\nSHIFT + WHEEL  SELECT PRODUCT\nR  NEXT PAGE\n\nRELEASE LMB TO SELECT"),
                TEXT("조작법\n좌클릭 유지  제어 메뉴\n우클릭  스캔\nSHIFT + 휠  제품 선택\nR  다음 페이지\n\n좌클릭을 놓아 선택"))
            : ItemScannerLocalization::String(
                TEXT("NO MATCHES\nCHANGE TARGET OR FILTERS\nTHEN RMB TO RESCAN"),
                TEXT("검색 결과 없음\n대상 또는 필터를 변경한 뒤\n우클릭으로 다시 스캔")),27,Dim);
    }
    for (int32 I=0;I<RowMetrics.Num() && Matches.IsValidIndex(Start+I);++I)
    {
        const auto& R=Matches[Start+I];
        RowMetrics[I]->SetText(FText::FromString(R.bIsValid?FString::Printf(TEXT("%.0fm  H%+.0fm"),R.CurrentDistanceMeters,R.HeightDifferenceMeters):ItemScannerLocalization::Choose(TEXT("REMOVED"),TEXT("제거됨"))));
        RowArrows[I]->SetRenderTransformAngle(R.RelativeYawDegrees);
        RowArrows[I]->SetRenderOpacity(R.bIsValid?1.f:0.2f);
    }
    const int32 PageCount=FMath::Max(1,FMath::DivideAndRoundUp(Matches.Num(),PageSize));
    Footer->SetText(FText::FromString(bKorean
        ? FString::Printf(TEXT("결과 %d개 | 페이지 %d/%d\n우클릭 스캔  ·  좌클릭 메뉴  ·  R 페이지"),Matches.Num(),Equipment->GetResultPage()+1,PageCount)
        : FString::Printf(TEXT("%d RESULTS | PAGE %d/%d\nRMB SCAN  ·  LMB MENU  ·  R PAGE"),Matches.Num(),Equipment->GetResultPage()+1,PageCount)));
}
void UItemScannerEquipmentScreen::AnimateProductStep(int32 Direction, float Duration)
{
    ProductScrollOffset=FMath::Sign(Direction)*68.f;
    ProductScrollSpeed=68.f/FMath::Max(Duration,0.025f);
    AdvanceProductAnimation(0.f);
}
bool UItemScannerEquipmentScreen::AdvanceProductAnimation(float DeltaSeconds)
{
    const bool bMoving=!FMath::IsNearlyZero(ProductScrollOffset);
    ProductScrollOffset=FMath::FInterpConstantTo(ProductScrollOffset,0.f,DeltaSeconds,ProductScrollSpeed);
    for (int32 I=0;I<ProductLines.Num();++I)
    {
        ProductLines[I]->SetRenderTranslation(FVector2D(0,ProductScrollOffset));
        const float Distance=FMath::Abs((I-2)*68.f+ProductScrollOffset)/68.f;
        ProductLines[I]->SetRenderOpacity(FMath::Lerp(1.f,0.48f,FMath::Clamp(Distance,0.f,1.f)));
    }
    return bMoving;
}
