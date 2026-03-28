#ifndef SLYMAIL_BBS_COLORS_H
#define SLYMAIL_BBS_COLORS_H

// BBS Color/Attribute Code Support
//
// Parses color codes from various BBS software packages and converts
// them to TermAttr for terminal display. Supported formats:
//
//   - ANSI escape sequences (ESC[...m)        -- always enabled
//   - Synchronet Ctrl-A codes (\x01 + attr)   -- toggle
//   - WWIV heart codes (\x03 + digit)         -- toggle
//   - PCBoard/Wildcat @X## codes              -- toggle
//   - Celerity pipe codes (|<letter>)          -- toggle
//   - Renegade pipe codes (|## two-digit)      -- toggle
//
// Strategy: All BBS codes are parsed inline and converted to TermAttr
// segments for rendering. The parseBBSColors() function produces a
// vector of text segments with associated colors.

#include "terminal.h"
#include <string>
#include <vector>

// Per-system enable flags for attribute code parsing.
// ANSI is always enabled and has no flag.
struct AttrCodeFlags
{
    bool synchronet = true; // Synchronet Ctrl-A codes
    bool wwiv       = true; // WWIV heart codes
    bool celerity   = true; // Celerity pipe codes (|letter)
    bool renegade   = true; // Renegade pipe codes (|00-|31)
    bool pcboard    = true; // PCBoard/Wildcat @X## codes
};

// A segment of text with a specific color attribute
struct ColoredSegment
{
    std::string text;
    TermAttr    attr;
};

// Parse a line of text containing BBS color codes and produce colored segments.
// ANSI escape codes are always processed.
// Other code types are processed according to the flags parameter.
// currentAttr is the starting attribute state (modified as codes are processed).
std::vector<ColoredSegment> parseBBSColors(const std::string& line,
                                            TermAttr& currentAttr,
                                            const AttrCodeFlags& flags = AttrCodeFlags());

// Strip all recognized BBS color codes from a string, returning plain text.
std::string stripBBSColors(const std::string& line,
                           const AttrCodeFlags& flags = AttrCodeFlags());

// Convert a TermAttr to an ANSI SGR escape sequence string.
std::string termAttrToAnsi(const TermAttr& attr);

// Parse a single ANSI SGR parameter and update the attribute accordingly.
// Handles codes 0 (reset), 1 (bold), 5 (blink), 30-37 (fg), 40-47 (bg).
void applyAnsiSgrParam(int param, TermAttr& attr);

#endif // SLYMAIL_BBS_COLORS_H
