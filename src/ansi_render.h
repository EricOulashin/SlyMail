#ifndef SLYMAIL_ANSI_RENDER_H
#define SLYMAIL_ANSI_RENDER_H

// ANSI Screen Renderer — renders raw ANSI text into a virtual screen buffer.
// Used by the message reader to correctly display ANSI art that uses cursor
// positioning, screen clearing, and other terminal control codes.

#include "terminal.h"
#include <string>
#include <vector>

// A single cell in the virtual screen buffer
struct AnsiCell
{
    char ch;         // Character (space if empty)
    TermAttr attr;   // Color/attribute for this cell

    AnsiCell() : ch(' '), attr(TC_WHITE, TC_BLACK, false) {}
};

// A rendered line from the ANSI screen buffer, with per-character attributes
struct AnsiLine
{
    std::vector<AnsiCell> cells;
};

// Check if a message body contains ANSI cursor positioning codes,
// indicating it's ANSI art that needs the screen renderer.
bool isAnsiArt(const std::string& body);

// Render raw ANSI text into a virtual screen buffer.
// screenWidth is the width of the virtual screen (typically terminal width).
// Returns a vector of AnsiLine, one per row of the rendered output.
// Trailing empty rows are trimmed.
std::vector<AnsiLine> renderAnsiToScreen(const std::string& body,
                                          int screenWidth);

#endif // SLYMAIL_ANSI_RENDER_H
