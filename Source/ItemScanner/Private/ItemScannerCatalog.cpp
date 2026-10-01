#include "ItemScannerCatalog.h"
#include "FGRecipeManager.h"
#include "FGRecipe.h"
#include "Resources/FGItemDescriptor.h"
#include "Resources/FGBuildDescriptor.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Culture.h"

bool UItemScannerCatalog::EnsureBuilt(UObject* WorldContext)
{
    if (!bBuilt)
    {
        AFGRecipeManager* Recipes = AFGRecipeManager::Get(WorldContext);
        if (!IsValid(Recipes) || Recipes->GetAllRecipes().IsEmpty()) return false;
        TSet<UClass*> Seen;
        auto Add = [this, &Seen](const TArray<FItemAmount>& Amounts)
        {
            for (const FItemAmount& Amount : Amounts)
            {
                UClass* Class = Amount.ItemClass.Get();
                if (IsValid(Class) && Class->IsChildOf(UFGItemDescriptor::StaticClass()) &&
                    !Class->IsChildOf(UFGBuildDescriptor::StaticClass()) &&
                    UFGItemDescriptor::GetForm(Class)==EResourceForm::RF_SOLID &&
                    UFGItemDescriptor::GetStackSize(Class)>0 &&
                    !Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists) &&
                    !Seen.Contains(Class))
                {
                    Seen.Add(Class); Items.Add(Class);
                }
            }
        };
        for (auto Recipe : Recipes->GetAllRecipes())
        {
            if (!Recipe) continue;
            Add(UFGRecipe::GetIngredients(WorldContext, Recipe));
            Add(UFGRecipe::GetProducts(Recipe));
        }
        bBuilt = !Items.IsEmpty();
    }
    const FString Culture = FInternationalization::Get().GetCurrentCulture()->GetName();
    if (bBuilt && Culture != SortedCulture)
    {
        Items.Sort([](const TSubclassOf<UFGItemDescriptor>& A, const TSubclassOf<UFGItemDescriptor>& B)
        {
            const int32 Order = UFGItemDescriptor::GetItemName(A).CompareTo(UFGItemDescriptor::GetItemName(B));
            return Order == 0 ? A->GetPathName() < B->GetPathName() : Order < 0;
        });
        SortedCulture = Culture;
    }
    return bBuilt;
}
