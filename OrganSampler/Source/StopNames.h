#pragma once

#include <JuceHeader.h>

//==============================================================================
// Stop names made by the program (the tree scan's row names, the aliases from
// folder names, the Hauptwerk import's aliases) - never a name the user typed.
namespace StopNames
{
    // The first letter of the stop name proper after a division abbreviation
    // is a capital (0.8.135, the user's rule): "P. trombone 16" -> "P. Trombone
    // 16", "3 geigenpr 8" -> "3 Geigenpr 8", "Rec: hautbois 8" -> "Rec: Hautbois
    // 8". An abbreviation is the first token - up to five letters or digits
    // ending in '.' or ':' ("Ped." "GO." "II." "Rec:"), a chain of such pieces
    // ("G.O." "R.P."), a bare number or roman numeral ("3", "II") - or, since
    // 0.8.200 (the user: the manuals' marks too, "HW, BW, RW, BP, Pos., Réc.,
    // GO, G.O., CH. a tak podobně"), a bare mark of a manual: a token of two
    // to five CAPITAL letters ("HW" "BW" "RW" "BP" "GO" "CH" "SW" "OW" "UW" "RP"
    // "GT" "PED") or a known word in any case ("Pos", "Rec", "Réc", "Ped", "Man",
    // "Gr", "Gt", "Hw", "Sw" ...) - followed by a space.
    inline juce::String capitaliseAfterDivisionPrefix(juce::String name)
    {
        const int length = name.length();
        int i = 0;
        while (i < length && name[i] == ' ') ++i;
        bool marked = false;
        juce::String stem;
        for (;;)
        {
            const int start = i;
            while (i < length && juce::CharacterFunctions::isLetterOrDigit(name[i])) ++i;
            stem = name.substring(start, i);
            if (stem.isEmpty() || stem.length() > 5) return name;
            if (i < length && (name[i] == '.' || name[i] == ':'))
            {
                marked = true;
                ++i;
                if (i < length && juce::CharacterFunctions::isLetter(name[i])) continue;   // "G.O." - the next piece
            }
            break;
        }
        if (i >= length || name[i] != ' ') return name;   // "P.trombone", a bare word: no
        if (!marked)
        {
            const auto lower = stem.toLowerCase();
            const bool number = stem.containsOnly("0123456789") && stem.length() <= 2;
            const bool roman = lower.containsOnly("ivx") && lower.length() <= 4;
            bool capitals = stem.length() >= 2;
            for (int c = 0; c < stem.length() && capitals; ++c)
                if (!juce::CharacterFunctions::isUpperCase(stem[c])) capitals = false;
            static const char* const words[] = { "pos", "rec", "r\xc3\xa9" "c", "ped", "pd", "man", "gr", "gt", "go", "hw", "ow", "uw", "bw", "rw", "sw", "ch", "bp", "rp", "pedal", "recit", "r\xc3\xa9" "cit", "swell", "great", "choir", "solo", "echo", "positif" };
            bool word = false;
            for (const char* w : words) if (lower == juce::String::fromUTF8(w)) { word = true; break; }
            if (!number && !roman && !capitals && !word) return name;
        }
        while (i < length && name[i] == ' ') ++i;
        if (i >= length) return name;
        const juce::juce_wchar c = name[i];
        if (!juce::CharacterFunctions::isLowerCase(c)) return name;
        return name.substring(0, i) + juce::String::charToString(juce::CharacterFunctions::toUpperCase(c)) + name.substring(i + 1);
    }

    // The rule tried on known names (--kerneltest prints it; "MISMATCH" fails the CI).
    inline juce::String selfCheck()
    {
        struct Case { const char* in; const char* out; };
        static const Case cases[] = {
            { "P. trombone 16", "P. Trombone 16" }, { "3 geigenpr 8", "3 Geigenpr 8" }, { "Rec: hautbois 8", "Rec: Hautbois 8" },
            { "HW trombone 8", "HW Trombone 8" }, { "RW gedact 8", "RW Gedact 8" }, { "BP cornet", "BP Cornet" },
            { "G.O. trompette 8", "G.O. Trompette 8" }, { "CH. flute 4", "CH. Flute 4" }, { "Pos flute 4", "Pos Flute 4" },
            { "R\xc3\xa9" "c. hautbois 8", "R\xc3\xa9" "c. Hautbois 8" }, { "II. principal 8", "II. Principal 8" }, { "hw flute 8", "hw Flute 8" },
            { "Flute 8", "Flute 8" }, { "flute 8", "flute 8" }, { "Bourdon 16", "Bourdon 16" }, { "Terz 1 3/5", "Terz 1 3/5" },
            { "A trumpet", "A trumpet" }, { "P.trombone", "P.trombone" }, { "Mixtur IV", "Mixtur IV" }, { "HW", "HW" }
        };
        juce::String report;
        int bad = 0;
        for (const auto& c : cases)
        {
            const juce::String got = capitaliseAfterDivisionPrefix(juce::String::fromUTF8(c.in));
            if (got != juce::String::fromUTF8(c.out)) { ++bad; report << "  \"" << juce::String::fromUTF8(c.in) << "\" -> \"" << got << "\" expected \"" << juce::String::fromUTF8(c.out) << "\"\n"; }
        }
        return "Stop name capitals after a division or manual mark: " + juce::String((int)std::size(cases)) + " cases -> " + (bad == 0 ? juce::String("OK") : juce::String(bad) + " MISMATCH") + "\n" + report;
    }
}
