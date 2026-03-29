#include "theme.h"
#include "terminal.h"

#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cctype>

using std::string;
using std::vector;
using std::map;

// ============================================================
// Synchronet attribute string parser
// ============================================================

TermAttr parseSyncAttr(const string& attrStr)
{
    // Synchronet Ctrl-A codes:
    // https://wiki.synchro.net/custom:ctrl-a_codes

    int fg = TC_WHITE;
    int bg = TC_BLACK;
    bool bright = false;

    for (const char c : attrStr)
    {
        switch (c)
        {
            case 'n': case 'N':
                fg = TC_WHITE;
                bg = TC_BLACK;
                bright = false;
                break;
            case 'k': case 'K':
                fg = TC_BLACK;
                break;
            case 'r': case 'R':
                fg = TC_RED;
                break;
            case 'g': case 'G':
                fg = TC_GREEN;
                break;
            case 'y': case 'Y':
                fg = TC_YELLOW;
                break;
            case 'b': case 'B':
                fg = TC_BLUE;
                break;
            case 'm': case 'M':
                fg = TC_MAGENTA;
                break;
            case 'c': case 'C':
                fg = TC_CYAN;
                break;
            case 'w': case 'W':
                fg = TC_WHITE;
                break;
            case 'h': case 'H':
                bright = true;
                break;
            case '0':
                bg = TC_BLACK;
                break;
            case '1':
                bg = TC_RED;
                break;
            case '2':
                bg = TC_GREEN;
                break;
            case '3':
                bg = TC_YELLOW;
                break;
            case '4':
                bg = TC_BLUE;
                break;
            case '5':
                bg = TC_MAGENTA;
                break;
            case '6':
                bg = TC_CYAN;
                break;
            case '7':
                bg = TC_WHITE;
                break;
            default:
                break;
        }
    }

    return TermAttr(fg, bg, bright);
}

// ============================================================
// INI file reader
// ============================================================

map<string, string> readIniFile(const string& path)
{
    map<string, string> kvMap;
    std::ifstream inFile(path);
    if (!inFile.is_open())
    {
        return kvMap;
    }

    string line;
    while (std::getline(inFile, line))
    {
        // Trim leading whitespace
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == string::npos)
        {
            continue;
        }
        line = line.substr(start);

        // Skip comments and section headers
        if (line.empty() || line[0] == ';' || line[0] == '#' || line[0] == '[')
        {
            continue;
        }

        // Find '=' separator
        size_t eqPos = line.find('=');
        if (eqPos == string::npos)
        {
            continue;
        }

        string key = line.substr(0, eqPos);
        string val = line.substr(eqPos + 1);

        // Trim trailing whitespace from key
        size_t keyEnd = key.find_last_not_of(" \t");
        if (keyEnd != string::npos)
        {
            key = key.substr(0, keyEnd + 1);
        }

        // Trim whitespace from value
        size_t valStart = val.find_first_not_of(" \t");
        if (valStart != string::npos)
        {
            val = val.substr(valStart);
        }
        else
        {
            val = "";
        }
        size_t valEnd = val.find_last_not_of(" \t\r\n");
        if (valEnd != string::npos)
        {
            val = val.substr(0, valEnd + 1);
        }

        kvMap[key] = val;
    }

    return kvMap;
}

// Helper: look up a key in the map and parse it, or return a default
TermAttr getAttrOrDefault(const map<string, string>& kvMap,
                          const string& key,
                          const TermAttr& defaultAttr)
{
    auto it = kvMap.find(key);
    if (it != kvMap.end() && !it->second.empty())
    {
        return parseSyncAttr(it->second);
    }
    return defaultAttr;
}

// ============================================================
// Load Ice Theme
// ============================================================

IceTheme loadIceTheme(const string& path)
{
    IceTheme theme;
    auto kv = readIniFile(path);

    theme.borderColor1              = getAttrOrDefault(kv, "BorderColor1",              theme.borderColor1);
    theme.borderColor2              = getAttrOrDefault(kv, "BorderColor2",              theme.borderColor2);
    theme.quoteLineColor            = getAttrOrDefault(kv, "QuoteLineColor",            theme.quoteLineColor);
    theme.keyInfoLabelColor         = getAttrOrDefault(kv, "KeyInfoLabelColor",         theme.keyInfoLabelColor);

    theme.topInfoBkgColor           = getAttrOrDefault(kv, "TopInfoBkgColor",           theme.topInfoBkgColor);
    theme.topLabelColor             = getAttrOrDefault(kv, "TopLabelColor",             theme.topLabelColor);
    theme.topLabelColonColor        = getAttrOrDefault(kv, "TopLabelColonColor",        theme.topLabelColonColor);
    theme.topToColor                = getAttrOrDefault(kv, "TopToColor",                theme.topToColor);
    theme.topFromColor              = getAttrOrDefault(kv, "TopFromColor",              theme.topFromColor);
    theme.topSubjectColor           = getAttrOrDefault(kv, "TopSubjectColor",           theme.topSubjectColor);
    theme.topTimeColor              = getAttrOrDefault(kv, "TopTimeColor",              theme.topTimeColor);
    theme.topTimeLeftColor          = getAttrOrDefault(kv, "TopTimeLeftColor",          theme.topTimeLeftColor);
    theme.editMode                  = getAttrOrDefault(kv, "EditMode",                  theme.editMode);

    theme.quoteWinText              = getAttrOrDefault(kv, "QuoteWinText",              theme.quoteWinText);
    theme.quoteLineHighlightColor   = getAttrOrDefault(kv, "QuoteLineHighlightColor",   theme.quoteLineHighlightColor);
    theme.quoteWinBorderTextColor   = getAttrOrDefault(kv, "QuoteWinBorderTextColor",   theme.quoteWinBorderTextColor);

    theme.selectedOptionBorderColor   = getAttrOrDefault(kv, "SelectedOptionBorderColor",   theme.selectedOptionBorderColor);
    theme.selectedOptionTextColor     = getAttrOrDefault(kv, "SelectedOptionTextColor",     theme.selectedOptionTextColor);
    theme.unselectedOptionBorderColor = getAttrOrDefault(kv, "UnselectedOptionBorderColor", theme.unselectedOptionBorderColor);
    theme.unselectedOptionTextColor   = getAttrOrDefault(kv, "UnselectedOptionTextColor",   theme.unselectedOptionTextColor);

    theme.listBoxBorder             = getAttrOrDefault(kv, "listBoxBorder",             theme.listBoxBorder);
    theme.listBoxBorderText         = getAttrOrDefault(kv, "listBoxBorderText",         theme.listBoxBorderText);
    theme.listBoxItemText           = getAttrOrDefault(kv, "listBoxItemText",           theme.listBoxItemText);
    theme.listBoxItemHighlight      = getAttrOrDefault(kv, "listBoxItemHighlight",      theme.listBoxItemHighlight);

    return theme;
}

// ============================================================
// Load DCT Theme
// ============================================================

DctTheme loadDctTheme(const string& path)
{
    DctTheme theme;
    auto kv = readIniFile(path);

    theme.topBorderColor1           = getAttrOrDefault(kv, "TopBorderColor1",           theme.topBorderColor1);
    theme.topBorderColor2           = getAttrOrDefault(kv, "TopBorderColor2",           theme.topBorderColor2);
    theme.editAreaBorderColor1      = getAttrOrDefault(kv, "EditAreaBorderColor1",      theme.editAreaBorderColor1);
    theme.editAreaBorderColor2      = getAttrOrDefault(kv, "EditAreaBorderColor2",      theme.editAreaBorderColor2);
    theme.editModeBrackets          = getAttrOrDefault(kv, "EditModeBrackets",          theme.editModeBrackets);
    theme.editMode                  = getAttrOrDefault(kv, "EditMode",                  theme.editMode);

    theme.topLabelColor             = getAttrOrDefault(kv, "TopLabelColor",             theme.topLabelColor);
    theme.topLabelColonColor        = getAttrOrDefault(kv, "TopLabelColonColor",        theme.topLabelColonColor);
    theme.topFromColor              = getAttrOrDefault(kv, "TopFromColor",              theme.topFromColor);
    theme.topFromFillColor          = getAttrOrDefault(kv, "TopFromFillColor",          theme.topFromFillColor);
    theme.topToColor                = getAttrOrDefault(kv, "TopToColor",                theme.topToColor);
    theme.topToFillColor            = getAttrOrDefault(kv, "TopToFillColor",            theme.topToFillColor);
    theme.topSubjColor              = getAttrOrDefault(kv, "TopSubjColor",              theme.topSubjColor);
    theme.topSubjFillColor          = getAttrOrDefault(kv, "TopSubjFillColor",          theme.topSubjFillColor);
    theme.topAreaColor              = getAttrOrDefault(kv, "TopAreaColor",              theme.topAreaColor);
    theme.topAreaFillColor          = getAttrOrDefault(kv, "TopAreaFillColor",          theme.topAreaFillColor);
    theme.topTimeColor              = getAttrOrDefault(kv, "TopTimeColor",              theme.topTimeColor);
    theme.topTimeFillColor          = getAttrOrDefault(kv, "TopTimeFillColor",          theme.topTimeFillColor);
    theme.topTimeLeftColor          = getAttrOrDefault(kv, "TopTimeLeftColor",          theme.topTimeLeftColor);
    theme.topTimeLeftFillColor      = getAttrOrDefault(kv, "TopTimeLeftFillColor",      theme.topTimeLeftFillColor);
    theme.topInfoBracketColor       = getAttrOrDefault(kv, "TopInfoBracketColor",       theme.topInfoBracketColor);

    theme.quoteWinText              = getAttrOrDefault(kv, "QuoteWinText",              theme.quoteWinText);
    theme.quoteLineHighlightColor   = getAttrOrDefault(kv, "QuoteLineHighlightColor",   theme.quoteLineHighlightColor);
    theme.quoteWinBorderTextColor   = getAttrOrDefault(kv, "QuoteWinBorderTextColor",   theme.quoteWinBorderTextColor);
    theme.quoteWinBorderColor       = getAttrOrDefault(kv, "QuoteWinBorderColor",       theme.quoteWinBorderColor);

    theme.bottomHelpBrackets        = getAttrOrDefault(kv, "BottomHelpBrackets",        theme.bottomHelpBrackets);
    theme.bottomHelpKeys            = getAttrOrDefault(kv, "BottomHelpKeys",            theme.bottomHelpKeys);
    theme.bottomHelpFill            = getAttrOrDefault(kv, "BottomHelpFill",            theme.bottomHelpFill);
    theme.bottomHelpKeyDesc         = getAttrOrDefault(kv, "BottomHelpKeyDesc",         theme.bottomHelpKeyDesc);

    theme.textBoxBorder             = getAttrOrDefault(kv, "TextBoxBorder",             theme.textBoxBorder);
    theme.textBoxBorderText         = getAttrOrDefault(kv, "TextBoxBorderText",         theme.textBoxBorderText);
    theme.textBoxInnerText          = getAttrOrDefault(kv, "TextBoxInnerText",          theme.textBoxInnerText);
    theme.yesNoBoxBrackets          = getAttrOrDefault(kv, "YesNoBoxBrackets",          theme.yesNoBoxBrackets);
    theme.yesNoBoxYesNoText         = getAttrOrDefault(kv, "YesNoBoxYesNoText",         theme.yesNoBoxYesNoText);

    theme.listBoxBorder             = getAttrOrDefault(kv, "listBoxBorder",             theme.listBoxBorder);
    theme.listBoxBorderText         = getAttrOrDefault(kv, "listBoxBorderText",         theme.listBoxBorderText);
    theme.listBoxItemText           = getAttrOrDefault(kv, "listBoxItemText",           theme.listBoxItemText);
    theme.listBoxItemHighlight      = getAttrOrDefault(kv, "listBoxItemHighlight",      theme.listBoxItemHighlight);

    return theme;
}

// ============================================================
// Random two-color border drawing
// ============================================================

void drawRandomTwoColorLine(int row, int startCol, const vector<int>& chars,
                            const TermAttr& color1, const TermAttr& color2)
{
    for (size_t i = 0; i < chars.size(); ++i)
    {
        const TermAttr& chosen = (std::rand() % 2 == 0) ? color1 : color2;
        g_term->setAttr(chosen);
        g_term->putCP437(row, startCol + static_cast<int>(i), chars[i]);
    }
}

void drawRandomTwoColorHLine(int row, int col, int len,
                             const TermAttr& color1, const TermAttr& color2)
{
    for (int i = 0; i < len; ++i)
    {
        const TermAttr& chosen = (std::rand() % 2 == 0) ? color1 : color2;
        g_term->setAttr(chosen);
        g_term->putCP437(row, col + i, CP437_BOX_DRAWINGS_HORIZONTAL_SINGLE);
    }
}
