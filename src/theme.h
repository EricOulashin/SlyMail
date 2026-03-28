#ifndef SLYMAIL_THEME_H
#define SLYMAIL_THEME_H

// SlyMail Theme Loading
//
// Parses Synchronet-style color attribute strings from INI theme files
// and populates Ice-style and DCT-style theme structs.

#include "terminal.h"

#include <string>
#include <vector>
#include <map>

// ============================================================
// Synchronet attribute string parser
// ============================================================
// Parses strings like "nbh", "n4hc", "nhw" into TermAttr.
//
// Format:
//   n = normal (reset to defaults), always appears first if present
//   Foreground letters: k=black r=red g=green y=yellow b=blue m=magenta c=cyan w=white
//   h = high/bright
//   Background digits: 0=black 1=red 2=green 3=yellow/brown 4=blue 5=magenta 6=cyan 7=lightgray
//
std::vector<TermAttr> parseSyncAttr(const std::string& attrStr);

// ============================================================
// INI file reader (simple key=value, ignoring comments/sections)
// ============================================================
std::map<std::string, std::string> readIniFile(const std::string& path);

// Helper: look up a key in the map and parse it, or return a default
std::vector<TermAttr> getAttrOrDefault(const std::map<std::string, std::string>& kvMap,
                                       const std::string& key,
                                       const std::vector<TermAttr>& defaultAttrs);

// ============================================================
// Ice Theme (matches EditorIceColors_BlueIce.ini)
// ============================================================
struct IceTheme
{
    std::vector<TermAttr> borderColor1;
    std::vector<TermAttr> borderColor2;
    std::vector<TermAttr> quoteLineColor;
    std::vector<TermAttr> keyInfoLabelColor;

    // Top informational area
    std::vector<TermAttr> topInfoBkgColor;
    std::vector<TermAttr> topLabelColor;
    std::vector<TermAttr> topLabelColonColor;
    std::vector<TermAttr> topToColor;
    std::vector<TermAttr> topFromColor;
    std::vector<TermAttr> topSubjectColor;
    std::vector<TermAttr> topTimeColor;
    std::vector<TermAttr> topTimeLeftColor;
    std::vector<TermAttr> editMode;

    // Quote window
    std::vector<TermAttr> quoteWinText;
    std::vector<TermAttr> quoteLineHighlightColor;
    std::vector<TermAttr> quoteWinBorderTextColor;

    // Multi-choice options
    std::vector<TermAttr> selectedOptionBorderColor;
    std::vector<TermAttr> selectedOptionTextColor;
    std::vector<TermAttr> unselectedOptionBorderColor;
    std::vector<TermAttr> unselectedOptionTextColor;

    // List boxes
    std::vector<TermAttr> listBoxBorder;
    std::vector<TermAttr> listBoxBorderText;
    std::vector<TermAttr> listBoxItemText;
    std::vector<TermAttr> listBoxItemHighlight;

    // Initializes the theme to default values
    void initDefaults()
    {
        borderColor1 = parseSyncAttr("nb");
        borderColor2 = parseSyncAttr("nbh");
        quoteLineColor = parseSyncAttr("nc");
        keyInfoLabelColor = parseSyncAttr("ch");

        // Top informational area
        topInfoBkgColor = parseSyncAttr("4");
        topLabelColor = parseSyncAttr("ch");
        topLabelColonColor = parseSyncAttr("bh");
        topToColor = parseSyncAttr("wh");
        topFromColor = parseSyncAttr("wh");
        topSubjectColor = parseSyncAttr("wh");
        topTimeColor = parseSyncAttr("wh");
        topTimeLeftColor = parseSyncAttr("gh");
        editMode = parseSyncAttr("ch");

        // Quote window
        quoteWinText = parseSyncAttr("nhw");
        quoteLineHighlightColor = parseSyncAttr("4hc");
        quoteWinBorderTextColor = parseSyncAttr("nch");

        // Multi-choice options
        selectedOptionBorderColor = parseSyncAttr("nbh4");
        selectedOptionTextColor = parseSyncAttr("nch4");
        unselectedOptionBorderColor = parseSyncAttr("nb");
        unselectedOptionTextColor = parseSyncAttr("nw");

        // List boxes
        listBoxBorder = parseSyncAttr("nb");
        listBoxBorderText = parseSyncAttr("nbh");
        listBoxItemText = parseSyncAttr("nc");
        listBoxItemHighlight = parseSyncAttr("n7b");
    }
};

// ============================================================
// DCT Theme (matches EditorDCTColors_Default.ini)
// ============================================================
struct DctTheme
{
    // Border colors
    std::vector<TermAttr> topBorderColor1;
    std::vector<TermAttr> topBorderColor2;
    std::vector<TermAttr> editAreaBorderColor1;
    std::vector<TermAttr> editAreaBorderColor2;
    std::vector<TermAttr> editModeBrackets;
    std::vector<TermAttr> editMode;

    // Top informational area
    std::vector<TermAttr> topLabelColor;
    std::vector<TermAttr> topLabelColonColor;
    std::vector<TermAttr> topFromColor;
    std::vector<TermAttr> topFromFillColor;
    std::vector<TermAttr> topToColor;
    std::vector<TermAttr> topToFillColor;
    std::vector<TermAttr> topSubjColor;
    std::vector<TermAttr> topSubjFillColor;
    std::vector<TermAttr> topAreaColor;
    std::vector<TermAttr> topAreaFillColor;
    std::vector<TermAttr> topTimeColor;
    std::vector<TermAttr> topTimeFillColor;
    std::vector<TermAttr> topTimeLeftColor;
    std::vector<TermAttr> topTimeLeftFillColor;
    std::vector<TermAttr> topInfoBracketColor;

    // Quote window
    std::vector<TermAttr> quoteWinText;
    std::vector<TermAttr> quoteLineHighlightColor;
    std::vector<TermAttr> quoteWinBorderTextColor;
    std::vector<TermAttr> quoteWinBorderColor;

    // Bottom help row
    std::vector<TermAttr> bottomHelpBrackets;
    std::vector<TermAttr> bottomHelpKeys;
    std::vector<TermAttr> bottomHelpFill;
    std::vector<TermAttr> bottomHelpKeyDesc;

    // Text boxes
    std::vector<TermAttr> textBoxBorder;
    std::vector<TermAttr> textBoxBorderText;
    std::vector<TermAttr> textBoxInnerText;
    std::vector<TermAttr> yesNoBoxBrackets;
    std::vector<TermAttr> yesNoBoxYesNoText;

    // List boxes
    std::vector<TermAttr> listBoxBorder;
    std::vector<TermAttr> listBoxBorderText;
    std::vector<TermAttr> listBoxItemText;
    std::vector<TermAttr> listBoxItemHighlight;

    // Initializes the theme to default values
    void initDefaults()
    {
        topBorderColor1 = parseSyncAttr("nr");
        topBorderColor2 = parseSyncAttr("mrh");
        editAreaBorderColor1 = parseSyncAttr("ng");
        editAreaBorderColor2 = parseSyncAttr("ngh");
        editModeBrackets = parseSyncAttr("nkh");
        editMode = parseSyncAttr("nw");

        topLabelColor = parseSyncAttr("nbh");
        topLabelColonColor = parseSyncAttr("nb");
        topFromColor = parseSyncAttr("nch");
        topFromFillColor = parseSyncAttr("nc");
        topToColor = parseSyncAttr("nch");
        topToFillColor = parseSyncAttr("nc");
        topSubjColor = parseSyncAttr("nwh");
        topSubjFillColor = parseSyncAttr("nw");
        topAreaColor = parseSyncAttr("ngh");
        topAreaFillColor = parseSyncAttr("ng");
        topTimeColor = parseSyncAttr("nyh");
        topTimeFillColor = parseSyncAttr("nr");
        topTimeLeftColor = parseSyncAttr("nyh");
        topTimeLeftFillColor = parseSyncAttr("nr");
        topInfoBracketColor = parseSyncAttr("nm");

        quoteWinText = parseSyncAttr("n7b");
        quoteLineHighlightColor = parseSyncAttr("nw");
        quoteWinBorderTextColor = parseSyncAttr("n7r");
        quoteWinBorderColor = parseSyncAttr("nk7");

        bottomHelpBrackets = parseSyncAttr("nkh");
        bottomHelpKeys = parseSyncAttr("nrh");
        bottomHelpFill = parseSyncAttr("nr");
        bottomHelpKeyDesc = parseSyncAttr("nc");

        textBoxBorder = parseSyncAttr("nk7");
        textBoxBorderText = parseSyncAttr("nr7");
        textBoxInnerText = parseSyncAttr("nb7");
        yesNoBoxBrackets = parseSyncAttr("nk7");
        yesNoBoxYesNoText = parseSyncAttr("nwh7");

        listBoxBorder = parseSyncAttr("ng");
        listBoxBorderText = parseSyncAttr("nbh");
        listBoxItemText = parseSyncAttr("nc");
        listBoxItemHighlight = parseSyncAttr("n7b");
    }
};

// ============================================================
// Load Ice Theme
// ============================================================
IceTheme loadIceTheme(const std::string& path);

// ============================================================
// Load DCT Theme
// ============================================================
DctTheme loadDctTheme(const std::string& path);

// ============================================================
// Random two-color border drawing
// ============================================================

void drawRandomTwoColorLine(int row, int startCol, const std::vector<int>& chars,
                            const TermAttr& color1, const TermAttr& color2);

void drawRandomTwoColorHLine(int row, int col, int len,
                             const TermAttr& color1, const TermAttr& color2);

#endif // SLYMAIL_THEME_H
