#include "ItemScannerRadialDialWidget.h"

#include "Rendering/DrawElements.h"
#include "Widgets/SLeafWidget.h"

namespace
{
    class SItemScannerRadialDial final : public SLeafWidget
    {
    public:
        SLATE_BEGIN_ARGS(SItemScannerRadialDial) {}
            SLATE_ATTRIBUTE(int32, Selection)
        SLATE_END_ARGS()

        void Construct(const FArguments& Args) { Selection=Args._Selection; }

        virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D(760.f); }

        virtual int32 OnPaint(const FPaintArgs&,const FGeometry& Geometry,const FSlateRect&,
            FSlateWindowElementList& Elements,int32 Layer,const FWidgetStyle& Style,bool) const override
        {
            const FVector2f Size(Geometry.GetLocalSize());
            const FVector2f Center=Size*0.5f;
            const float Unit=FMath::Min(Size.X,Size.Y)/760.f;
            const float Inner=135.f*Unit, Outer=315.f*Unit;
            const FPaintGeometry Paint=Geometry.ToPaintGeometry();
            const FLinearColor Tint=Style.GetColorAndOpacityTint();
            const int32 Selected=Selection.Get(INDEX_NONE);
            auto Point=[&](float Radius,float Degrees)
            {
                const float Radians=FMath::DegreesToRadians(Degrees);
                return Center+FVector2f(FMath::Sin(Radians),-FMath::Cos(Radians))*Radius;
            };
            auto Arc=[&](float Radius,float Begin,float End)
            {
                TArray<FVector2f> Points;
                constexpr int32 Segments=18;
                for (int32 I=0;I<=Segments;++I)
                    Points.Add(Point(Radius,FMath::Lerp(Begin,End,static_cast<float>(I)/Segments)));
                return Points;
            };

            // Paint each annular sector from concentric antialiased strokes.
            constexpr int32 SectorCount=7;
            constexpr float SectorAngle=360.f/SectorCount;
            for (int32 I=0;I<SectorCount;++I)
            {
                const float Mid=I*SectorAngle;
                const float Begin=Mid-SectorAngle*0.46f,End=Mid+SectorAngle*0.46f;
                const bool bSelected=I==Selected;
                const FLinearColor Fill=(bSelected
                    ? FLinearColor(0.96f,0.43f,0.06f,0.96f)
                    : FLinearColor(0.015f,0.025f,0.03f,0.91f))*Tint;
                for (float Radius=Inner+7.f*Unit;Radius<Outer;Radius+=13.f*Unit)
                    FSlateDrawElement::MakeLines(Elements,Layer,Paint,Arc(Radius,Begin,End),
                        ESlateDrawEffect::None,Fill,true,15.f*Unit);

                const FLinearColor Edge=(bSelected
                    ? FLinearColor(1.f,0.65f,0.18f,1.f)
                    : FLinearColor(0.9f,0.94f,0.95f,0.95f))*Tint;
                FSlateDrawElement::MakeLines(Elements,Layer+1,Paint,Arc(Inner,Begin,End),ESlateDrawEffect::None,Edge,true,3.f*Unit);
                FSlateDrawElement::MakeLines(Elements,Layer+1,Paint,Arc(Outer,Begin,End),ESlateDrawEffect::None,Edge,true,3.f*Unit);
                const TArray<FVector2f> Left={Point(Inner,Begin),Point(Outer,Begin)};
                const TArray<FVector2f> Right={Point(Inner,End),Point(Outer,End)};
                FSlateDrawElement::MakeLines(Elements,Layer+1,Paint,Left,ESlateDrawEffect::None,Edge,true,3.f*Unit);
                FSlateDrawElement::MakeLines(Elements,Layer+1,Paint,Right,ESlateDrawEffect::None,Edge,true,3.f*Unit);
            }

            // Direction chevron in the centre follows the selected sector.
            if (Selected>=0 && Selected<SectorCount)
            {
                const float Mid=Selected*SectorAngle;
                const FVector2f Tip=Point(105.f*Unit,Mid);
                const FVector2f Left=Point(72.f*Unit,Mid-17.f);
                const FVector2f Right=Point(72.f*Unit,Mid+17.f);
                const TArray<FVector2f> Chevron={Left,Tip,Right};
                FSlateDrawElement::MakeLines(Elements,Layer+2,Paint,Chevron,ESlateDrawEffect::None,
                    FLinearColor::White*Tint,true,9.f*Unit);
            }
            return Layer+2;
        }
    private:
        TAttribute<int32> Selection;
    };
}

void UItemScannerRadialDialWidget::SetSelection(int32 InSelection)
{
    Selection=InSelection;
    if (DialSlate) DialSlate->Invalidate(EInvalidateWidgetReason::Paint);
}

TSharedRef<SWidget> UItemScannerRadialDialWidget::RebuildWidget()
{
    TWeakObjectPtr<UItemScannerRadialDialWidget> WeakThis(this);
    return SAssignNew(DialSlate,SItemScannerRadialDial)
        .Selection_Lambda([WeakThis](){ return WeakThis.IsValid()?WeakThis->GetSelection():INDEX_NONE; });
}

void UItemScannerRadialDialWidget::ReleaseSlateResources(bool bReleaseChildren)
{
    Super::ReleaseSlateResources(bReleaseChildren);
    DialSlate.Reset();
}
