#pragma once

#include "Internationalization/Internationalization.h"

namespace ItemScannerLocalization
{
    inline bool IsKorean()
    {
        const FCulturePtr Language = FInternationalization::Get().GetCurrentLanguage();
        return Language.IsValid() && Language->GetTwoLetterISOLanguageName().Equals(TEXT("ko"), ESearchCase::IgnoreCase);
    }

    inline const TCHAR* Choose(const TCHAR* English, const TCHAR* Korean)
    {
        return IsKorean() ? Korean : English;
    }

    inline FString String(const TCHAR* English, const TCHAR* Korean)
    {
        return FString(Choose(English, Korean));
    }

    inline FText Text(const TCHAR* English, const TCHAR* Korean)
    {
        return FText::FromString(String(English, Korean));
    }
}
