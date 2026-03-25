#include "bbs_colors.h"

using std::string;
using std::vector;

// ============================================================
// Internal helpers
// ============================================================

// Synchronet Ctrl-A code background color digit to TermColor
static int syncBgDigitToColor(char ch)
{
    switch (ch)
    {
        case '0': return TC_BLACK;
        case '1': return TC_RED;
        case '2': return TC_GREEN;
        case '3': return TC_YELLOW;
        case '4': return TC_BLUE;
        case '5': return TC_MAGENTA;
        case '6': return TC_CYAN;
        case '7': return TC_WHITE;
        default:  return -1;
    }
}

// Synchronet Ctrl-A code foreground letter to TermColor
static int syncFgLetterToColor(char ch)
{
    switch (ch)
    {
        case 'k': case 'K': return TC_BLACK;
        case 'r': case 'R': return TC_RED;
        case 'g': case 'G': return TC_GREEN;
        case 'y': case 'Y': return TC_YELLOW;
        case 'b': case 'B': return TC_BLUE;
        case 'm': case 'M': return TC_MAGENTA;
        case 'c': case 'C': return TC_CYAN;
        case 'w': case 'W': return TC_WHITE;
        default:  return -1;
    }
}

// Apply a Synchronet Ctrl-A attribute character to a TermAttr
static bool applySyncAttr(char ch, TermAttr& attr)
{
    // Normal/reset
    if (ch == 'n' || ch == 'N' || ch == '-' || ch == '_')
    {
        attr = tAttr(TC_WHITE, TC_BLACK, false);
        return true;
    }
    // High intensity
    if (ch == 'h' || ch == 'H')
    {
        attr.bright = true;
        return true;
    }
    // Blink (we don't support blink, but consume the code)
    if (ch == 'i' || ch == 'I' || ch == 'f' || ch == 'F')
    {
        return true;
    }
    // Foreground color
    int fg = syncFgLetterToColor(ch);
    if (fg >= 0)
    {
        attr.fg = fg;
        return true;
    }
    // Background color
    int bg = syncBgDigitToColor(ch);
    if (bg >= 0)
    {
        attr.bg = bg;
        return true;
    }
    return false;
}

// ============================================================
// ANSI SGR parameter handling
// ============================================================

void applyAnsiSgrParam(int param, TermAttr& attr)
{
    if (param == 0)
    {
        attr = tAttr(TC_WHITE, TC_BLACK, false);
    }
    else if (param == 1)
    {
        attr.bright = true;
    }
    else if (param == 5 || param == 6)
    {
        // Blink - consume but don't apply
    }
    else if (param >= 30 && param <= 37)
    {
        attr.fg = param - 30;
    }
    else if (param >= 40 && param <= 47)
    {
        attr.bg = param - 40;
    }
    // Ignore 256-color and truecolor sequences (38;5;n, 48;5;n, etc.)
}

// Parse ANSI escape sequence starting at ESC[ and return number of chars consumed
// (including the ESC and [). Updates attr with any SGR parameters found.
// Returns 0 if not a valid/recognized sequence.
static size_t parseAnsiSequence(const string& line, size_t pos, TermAttr& attr)
{
    // pos points to ESC (0x1B)
    if (pos + 1 >= line.size() || line[pos + 1] != '[')
    {
        return 0;
    }

    size_t i = pos + 2; // after ESC[
    // Collect parameters
    vector<int> params;
    int currentParam = 0;
    bool hasParam = false;

    while (i < line.size())
    {
        char ch = line[i];
        if (ch >= '0' && ch <= '9')
        {
            currentParam = currentParam * 10 + (ch - '0');
            hasParam = true;
            ++i;
        }
        else if (ch == ';')
        {
            params.push_back(hasParam ? currentParam : 0);
            currentParam = 0;
            hasParam = false;
            ++i;
        }
        else if (ch >= 0x40 && ch <= 0x7E) // Final byte
        {
            if (hasParam)
            {
                params.push_back(currentParam);
            }
            ++i; // consume final byte

            // Only process SGR sequences (final byte 'm')
            if (ch == 'm')
            {
                if (params.empty())
                {
                    // ESC[m is the same as ESC[0m
                    applyAnsiSgrParam(0, attr);
                }
                else
                {
                    for (int p : params)
                    {
                        applyAnsiSgrParam(p, attr);
                    }
                }
            }
            // For non-SGR sequences, we consume them but don't change attr
            return i - pos;
        }
        else
        {
            // Invalid sequence character
            return 0;
        }
    }
    return 0; // Unterminated sequence
}

// ============================================================
// PCBoard @X## code handling
// ============================================================

static int hexDigitValue(char ch)
{
    if (ch >= '0' && ch <= '9') return ch - '0';
    if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
    return -1;
}

// PCBoard @X## format: first hex digit = background, second = foreground
// Bit 3 of foreground = high intensity
static bool parsePCBoardCode(char bgHex, char fgHex, TermAttr& attr)
{
    int bgVal = hexDigitValue(bgHex);
    int fgVal = hexDigitValue(fgHex);
    if (bgVal < 0 || fgVal < 0) return false;

    // Foreground: low 3 bits = color, bit 3 = bright
    attr.fg = fgVal & 0x07;
    attr.bright = (fgVal & 0x08) != 0;
    // Background: low 3 bits = color (bit 3 = blink, ignored)
    attr.bg = bgVal & 0x07;
    return true;
}

// ============================================================
// WWIV heart code handling
// ============================================================

static bool applyWWIVCode(char digit, TermAttr& attr)
{
    switch (digit)
    {
        case '0': attr = tAttr(TC_WHITE, TC_BLACK, false); break;        // Normal
        case '1': attr = tAttr(TC_CYAN, TC_BLACK, true); break;          // Bright cyan
        case '2': attr = tAttr(TC_YELLOW, TC_BLACK, true); break;        // Bright yellow
        case '3': attr = tAttr(TC_MAGENTA, TC_BLACK, false); break;      // Magenta
        case '4': attr = tAttr(TC_WHITE, TC_BLUE, true); break;          // Bright white on blue
        case '5': attr = tAttr(TC_GREEN, TC_BLACK, false); break;        // Green
        case '6': attr = tAttr(TC_RED, TC_BLACK, true); break;           // Bright red (blink ignored)
        case '7': attr = tAttr(TC_BLUE, TC_BLACK, true); break;          // Bright blue
        case '8': attr = tAttr(TC_BLUE, TC_BLACK, false); break;         // Blue
        case '9': attr = tAttr(TC_CYAN, TC_BLACK, false); break;         // Cyan
        default: return false;
    }
    return true;
}

// ============================================================
// Celerity pipe code handling
// ============================================================

static bool applyCelerityCode(char ch, TermAttr& attr)
{
    // Lowercase = normal intensity foreground on black
    switch (ch)
    {
        case 'k': attr = tAttr(TC_BLACK, TC_BLACK, false); return true;
        case 'b': attr = tAttr(TC_BLUE, TC_BLACK, false); return true;
        case 'g': attr = tAttr(TC_GREEN, TC_BLACK, false); return true;
        case 'c': attr = tAttr(TC_CYAN, TC_BLACK, false); return true;
        case 'r': attr = tAttr(TC_RED, TC_BLACK, false); return true;
        case 'm': attr = tAttr(TC_MAGENTA, TC_BLACK, false); return true;
        case 'y': attr = tAttr(TC_YELLOW, TC_BLACK, false); return true;
        case 'w': attr = tAttr(TC_WHITE, TC_BLACK, false); return true;
        // Dark/bright
        case 'd': attr = tAttr(TC_BLACK, TC_BLACK, true); return true;
        // Uppercase = high intensity foreground on black
        case 'B': attr = tAttr(TC_BLUE, TC_BLACK, true); return true;
        case 'G': attr = tAttr(TC_GREEN, TC_BLACK, true); return true;
        case 'C': attr = tAttr(TC_CYAN, TC_BLACK, true); return true;
        case 'R': attr = tAttr(TC_RED, TC_BLACK, true); return true;
        case 'M': attr = tAttr(TC_MAGENTA, TC_BLACK, true); return true;
        case 'Y': attr = tAttr(TC_YELLOW, TC_BLACK, true); return true;
        case 'W': attr = tAttr(TC_WHITE, TC_BLACK, true); return true;
        case 'S': return true; // Swap (not fully implemented - would need tracking)
        default: return false;
    }
}

// ============================================================
// Renegade pipe code handling
// ============================================================

static bool applyRenegadeCode(int code, TermAttr& attr)
{
    // 00-07: Normal foreground colors
    if (code >= 0 && code <= 7)
    {
        attr.fg = code;
        attr.bright = false;
        return true;
    }
    // 08-15: High intensity foreground colors
    if (code >= 8 && code <= 15)
    {
        attr.fg = code - 8;
        attr.bright = true;
        return true;
    }
    // 16-23: Background colors
    if (code >= 16 && code <= 23)
    {
        // Renegade bg order: black, blue, green, cyan, red, magenta, yellow, white
        static const int bgMap[] = { TC_BLACK, TC_BLUE, TC_GREEN, TC_CYAN,
                                     TC_RED, TC_MAGENTA, TC_YELLOW, TC_WHITE };
        attr.bg = bgMap[code - 16];
        return true;
    }
    // 24-31: Blinking (we set the color but ignore blink)
    if (code >= 24 && code <= 31)
    {
        static const int bgMap[] = { TC_BLACK, TC_BLUE, TC_GREEN, TC_CYAN,
                                     TC_RED, TC_MAGENTA, TC_YELLOW, TC_WHITE };
        attr.fg = TC_WHITE;
        attr.bright = false;
        attr.bg = bgMap[code - 24];
        return true;
    }
    return false;
}

// ============================================================
// Main parsing function
// ============================================================

vector<ColoredSegment> parseBBSColors(const string& line,
                                       TermAttr& currentAttr,
                                       const AttrCodeFlags& flags)
{
    vector<ColoredSegment> segments;
    string currentText;

    auto flushText = [&]()
    {
        if (!currentText.empty())
        {
            segments.push_back({currentText, currentAttr});
            currentText.clear();
        }
    };

    size_t i = 0;
    while (i < line.size())
    {
        uint8_t ch = static_cast<uint8_t>(line[i]);

        // --- ANSI escape sequence (always enabled) ---
        if (ch == 0x1B && i + 1 < line.size() && line[i + 1] == '[')
        {
            flushText();
            size_t consumed = parseAnsiSequence(line, i, currentAttr);
            if (consumed > 0)
            {
                i += consumed;
                continue;
            }
            // Not a valid sequence, output ESC as-is
        }

        // --- Synchronet Ctrl-A codes ---
        if (flags.synchronet && ch == 0x01 && i + 1 < line.size())
        {
            char next = line[i + 1];
            if (applySyncAttr(next, currentAttr))
            {
                flushText();
                i += 2;
                continue;
            }
            // Not a recognized Ctrl-A code, output as-is
        }

        // --- WWIV heart codes (Ctrl-C + digit) ---
        if (flags.wwiv && ch == 0x03 && i + 1 < line.size())
        {
            char next = line[i + 1];
            if (next >= '0' && next <= '9')
            {
                flushText();
                applyWWIVCode(next, currentAttr);
                i += 2;
                continue;
            }
        }

        // --- PCBoard/Wildcat @X## codes ---
        if (flags.pcboard && ch == '@' && i + 3 < line.size() &&
            (line[i + 1] == 'X' || line[i + 1] == 'x'))
        {
            char h1 = line[i + 2];
            char h2 = line[i + 3];
            if (hexDigitValue(h1) >= 0 && hexDigitValue(h2) >= 0)
            {
                flushText();
                parsePCBoardCode(h1, h2, currentAttr);
                i += 4;
                continue;
            }
        }

        // --- Pipe codes (Renegade and/or Celerity) ---
        // Renegade is checked first since its pattern (|NN) is more specific.
        if (ch == '|' && i + 1 < line.size())
        {
            if (flags.renegade && i + 2 < line.size())
            {
                char d1 = line[i + 1];
                char d2 = line[i + 2];
                if (d1 >= '0' && d1 <= '3' && d2 >= '0' && d2 <= '9')
                {
                    int code = (d1 - '0') * 10 + (d2 - '0');
                    if (code <= 31)
                    {
                        flushText();
                        applyRenegadeCode(code, currentAttr);
                        i += 3;
                        continue;
                    }
                }
            }
            if (flags.celerity)
            {
                char next = line[i + 1];
                if (applyCelerityCode(next, currentAttr))
                {
                    flushText();
                    i += 2;
                    continue;
                }
            }
        }

        // --- Regular character ---
        currentText += static_cast<char>(ch);
        ++i;
    }

    flushText();
    return segments;
}

// ============================================================
// Strip all BBS color codes
// ============================================================

string stripBBSColors(const string& line, const AttrCodeFlags& flags)
{
    TermAttr dummy = tAttr(TC_WHITE, TC_BLACK, false);
    auto segments = parseBBSColors(line, dummy, flags);
    string result;
    for (const auto& seg : segments)
    {
        result += seg.text;
    }
    return result;
}

// ============================================================
// Convert TermAttr to ANSI SGR sequence
// ============================================================

string termAttrToAnsi(const TermAttr& attr)
{
    // Always reset first, then set attributes
    string result = "\x1b[0";
    if (attr.bright)
    {
        result += ";1";
    }
    result += ";" + std::to_string(30 + attr.fg);
    if (attr.bg != TC_BLACK)
    {
        result += ";" + std::to_string(40 + attr.bg);
    }
    result += "m";
    return result;
}
