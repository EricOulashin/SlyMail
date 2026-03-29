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
TermAttr parseSyncAttr(const std::string& attrStr);

// ============================================================
// INI file reader (simple key=value, ignoring comments/sections)
// ============================================================
std::map<std::string, std::string> readIniFile(const std::string& path);

// Helper: look up a key in the map and parse it, or return a default
TermAttr getAttrOrDefault(const std::map<std::string, std::string>& kvMap,
                          const std::string& key,
                          const TermAttr& defaultAttr);

// ============================================================
// Ice Theme (matches EditorIceColors_BlueIce.ini)
// ============================================================
struct IceTheme
{
    TermAttr borderColor1 = parseSyncAttr("nb");
    TermAttr borderColor2 = parseSyncAttr("nbh");
    TermAttr quoteLineColor = parseSyncAttr("nc");
    TermAttr keyInfoLabelColor = parseSyncAttr("ch");

    // Top informational area
    TermAttr topInfoBkgColor = parseSyncAttr("4");
    TermAttr topLabelColor = parseSyncAttr("ch");
    TermAttr topLabelColonColor = parseSyncAttr("bh");
    TermAttr topToColor = parseSyncAttr("wh");
    TermAttr topFromColor = parseSyncAttr("wh");
    TermAttr topSubjectColor = parseSyncAttr("wh");
    TermAttr topTimeColor = parseSyncAttr("wh");
    TermAttr topTimeLeftColor = parseSyncAttr("gh");
    TermAttr editMode = parseSyncAttr("ch");

    // Quote window
    TermAttr quoteWinText = parseSyncAttr("nhw");
    TermAttr quoteLineHighlightColor = parseSyncAttr("4hc");
    TermAttr quoteWinBorderTextColor = parseSyncAttr("nch");

    // Multi-choice options
    TermAttr selectedOptionBorderColor = parseSyncAttr("nbh4");
    TermAttr selectedOptionTextColor = parseSyncAttr("nch4");
    TermAttr unselectedOptionBorderColor = parseSyncAttr("nb");
    TermAttr unselectedOptionTextColor = parseSyncAttr("nw");

    // List boxes
    TermAttr listBoxBorder = parseSyncAttr("nb");
    TermAttr listBoxBorderText = parseSyncAttr("nbh");
    TermAttr listBoxItemText = parseSyncAttr("nc");
    TermAttr listBoxItemHighlight = parseSyncAttr("n7b");
};

// ============================================================
// DCT Theme (matches EditorDCTColors_Default.ini)
// ============================================================
struct DctTheme
{
    // Border colors
    TermAttr topBorderColor1 = parseSyncAttr("nr");
    TermAttr topBorderColor2 = parseSyncAttr("mrh");
    TermAttr editAreaBorderColor1 = parseSyncAttr("ng");
    TermAttr editAreaBorderColor2 = parseSyncAttr("ngh");
    TermAttr editModeBrackets = parseSyncAttr("nkh");
    TermAttr editMode = parseSyncAttr("nw");

    // Top informational area
    TermAttr topLabelColor = parseSyncAttr("nbh");
    TermAttr topLabelColonColor = parseSyncAttr("nb");
    TermAttr topFromColor = parseSyncAttr("nch");
    TermAttr topFromFillColor = parseSyncAttr("nc");
    TermAttr topToColor = parseSyncAttr("nch");
    TermAttr topToFillColor = parseSyncAttr("nc");
    TermAttr topSubjColor = parseSyncAttr("nwh");
    TermAttr topSubjFillColor = parseSyncAttr("nw");
    TermAttr topAreaColor = parseSyncAttr("ngh");
    TermAttr topAreaFillColor = parseSyncAttr("ng");
    TermAttr topTimeColor = parseSyncAttr("nyh");
    TermAttr topTimeFillColor = parseSyncAttr("nr");
    TermAttr topTimeLeftColor = parseSyncAttr("nyh");
    TermAttr topTimeLeftFillColor = parseSyncAttr("nr");
    TermAttr topInfoBracketColor = parseSyncAttr("nm");

    // Quote window
    TermAttr quoteWinText = parseSyncAttr("n7b");
    TermAttr quoteLineHighlightColor = parseSyncAttr("nw");
    TermAttr quoteWinBorderTextColor = parseSyncAttr("n7r");
    TermAttr quoteWinBorderColor = parseSyncAttr("nk7");

    // Bottom help row
    TermAttr bottomHelpBrackets = parseSyncAttr("nkh");
    TermAttr bottomHelpKeys = parseSyncAttr("nrh");
    TermAttr bottomHelpFill = parseSyncAttr("nr");
    TermAttr bottomHelpKeyDesc = parseSyncAttr("nc");

    // Text boxes
    TermAttr textBoxBorder = parseSyncAttr("nk7");
    TermAttr textBoxBorderText = parseSyncAttr("nr7");
    TermAttr textBoxInnerText = parseSyncAttr("nb7");
    TermAttr yesNoBoxBrackets = parseSyncAttr("nk7");
    TermAttr yesNoBoxYesNoText = parseSyncAttr("nwh7");

    // List boxes
    TermAttr listBoxBorder = parseSyncAttr("ng");
    TermAttr listBoxBorderText = parseSyncAttr("nbh");
    TermAttr listBoxItemText = parseSyncAttr("nc");
    TermAttr listBoxItemHighlight = parseSyncAttr("n7b");
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
