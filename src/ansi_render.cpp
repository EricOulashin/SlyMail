// ansi_render.cpp — ANSI screen renderer.
// Processes raw ANSI text through a virtual terminal emulator to produce
// a screen buffer suitable for display in the message reader.

#include "ansi_render.h"
#include "bbs_colors.h"
#include <cstdint>
#include <algorithm>

using std::string;
using std::vector;

// ============================================================
// Detect ANSI art
// ============================================================

bool isAnsiArt(const string& body)
{
    // Look for cursor positioning sequences: ESC[H, ESC[nH, ESC[n;nH,
    // ESC[nA/B/C/D, ESC[J, ESC[K — these indicate ANSI art/screen layout
    for (size_t i = 0; i + 2 < body.size(); ++i)
    {
        if (static_cast<uint8_t>(body[i]) == 0x1B && body[i + 1] == '[')
        {
            // Scan to the final byte
            size_t j = i + 2;
            while (j < body.size() && ((body[j] >= '0' && body[j] <= '9') || body[j] == ';'))
            {
                ++j;
            }
            if (j < body.size())
            {
                char finalCh = body[j];
                // Cursor positioning and movement codes
                if (finalCh == 'H' || finalCh == 'f' ||  // Cursor position
                    finalCh == 'A' || finalCh == 'B' ||   // Cursor up/down
                    finalCh == 'C' || finalCh == 'D' ||   // Cursor forward/back
                    finalCh == 'J' || finalCh == 'K')     // Erase display/line
                {
                    return true;
                }
            }
        }
    }
    return false;
}

// ============================================================
// Virtual screen buffer
// ============================================================

struct VScreen
{
    int width;
    int cursorRow;
    int cursorCol;
    bool pendingWrap; // Deferred wrap: cursor at last column, wraps on next char
    TermAttr currentAttr;
    vector<vector<AnsiCell>> rows;

    // Saved cursor state (ESC[s / ESC[u)
    int savedRow;
    int savedCol;

    VScreen(int w) : width(w), cursorRow(0), cursorCol(0), pendingWrap(false),
                     currentAttr(TC_WHITE, TC_BLACK, false),
                     savedRow(0), savedCol(0)
    {
        // Start with one row
        ensureRow(0);
    }

    void ensureRow(int row)
    {
        while (static_cast<int>(rows.size()) <= row)
        {
            rows.push_back(vector<AnsiCell>(width));
        }
    }

    void putChar(char ch)
    {
        if (ch == '\n')
        {
            ++cursorRow;
            cursorCol = 0;
            pendingWrap = false;
            ensureRow(cursorRow);
            return;
        }
        if (ch == '\r')
        {
            cursorCol = 0;
            pendingWrap = false;
            return;
        }
        if (ch == '\t')
        {
            if (pendingWrap) { doWrap(); }
            int nextTab = ((cursorCol / 8) + 1) * 8;
            while (cursorCol < nextTab && cursorCol < width)
            {
                ensureRow(cursorRow);
                rows[cursorRow][cursorCol].ch = ' ';
                rows[cursorRow][cursorCol].attr = currentAttr;
                ++cursorCol;
            }
            if (cursorCol >= width)
            {
                cursorCol = width - 1;
                pendingWrap = true;
            }
            return;
        }
        // Allow all printable characters including CP437 high bytes (128-255)
        uint8_t uch = static_cast<uint8_t>(ch);
        if (uch < 32 && uch != 0x1B)
        {
            return; // Skip control chars (except ESC, handled elsewhere)
        }

        // Deferred wrap: if a previous character filled the last column,
        // wrap now before writing the new character
        if (pendingWrap)
        {
            doWrap();
        }

        ensureRow(cursorRow);
        if (cursorCol < width)
        {
            rows[cursorRow][cursorCol].ch = ch;
            rows[cursorRow][cursorCol].attr = currentAttr;
            ++cursorCol;
        }
        // If we just wrote to the last column, defer the wrap
        if (cursorCol >= width)
        {
            cursorCol = width - 1; // Keep cursor at last column
            pendingWrap = true;    // Will wrap on next printable character
        }
    }

    void doWrap()
    {
        cursorCol = 0;
        ++cursorRow;
        pendingWrap = false;
        ensureRow(cursorRow);
    }

    void moveCursor(int row, int col)
    {
        cursorRow = std::max(0, row);
        cursorCol = std::max(0, std::min(col, width - 1));
        pendingWrap = false;
        ensureRow(cursorRow);
    }

    void eraseDisplay(int mode)
    {
        if (mode == 2)
        {
            // Clear entire screen
            for (auto& row : rows)
            {
                for (auto& cell : row)
                {
                    cell.ch = ' ';
                    cell.attr = currentAttr;
                }
            }
            cursorRow = 0;
            cursorCol = 0;
        }
        else if (mode == 0)
        {
            // Clear from cursor to end
            ensureRow(cursorRow);
            for (int c = cursorCol; c < width; ++c)
            {
                rows[cursorRow][c].ch = ' ';
                rows[cursorRow][c].attr = currentAttr;
            }
            for (int r = cursorRow + 1; r < static_cast<int>(rows.size()); ++r)
            {
                for (auto& cell : rows[r])
                {
                    cell.ch = ' ';
                    cell.attr = currentAttr;
                }
            }
        }
        else if (mode == 1)
        {
            // Clear from start to cursor
            for (int r = 0; r < cursorRow; ++r)
            {
                for (auto& cell : rows[r])
                {
                    cell.ch = ' ';
                    cell.attr = currentAttr;
                }
            }
            ensureRow(cursorRow);
            for (int c = 0; c <= cursorCol && c < width; ++c)
            {
                rows[cursorRow][c].ch = ' ';
                rows[cursorRow][c].attr = currentAttr;
            }
        }
    }

    void eraseLine(int mode)
    {
        ensureRow(cursorRow);
        if (mode == 2)
        {
            // Clear entire line
            for (auto& cell : rows[cursorRow])
            {
                cell.ch = ' ';
                cell.attr = currentAttr;
            }
        }
        else if (mode == 0)
        {
            // Clear from cursor to end of line
            for (int c = cursorCol; c < width; ++c)
            {
                rows[cursorRow][c].ch = ' ';
                rows[cursorRow][c].attr = currentAttr;
            }
        }
        else if (mode == 1)
        {
            // Clear from start of line to cursor
            for (int c = 0; c <= cursorCol && c < width; ++c)
            {
                rows[cursorRow][c].ch = ' ';
                rows[cursorRow][c].attr = currentAttr;
            }
        }
    }
};

// ============================================================
// Parse CSI parameters
// ============================================================

struct CsiSequence
{
    vector<int> params;
    char finalByte;
    int length; // Total bytes consumed including ESC[
};

static CsiSequence parseCsi(const string& body, size_t pos)
{
    CsiSequence seq;
    seq.finalByte = 0;
    seq.length = 0;

    if (pos + 1 >= body.size() || body[pos + 1] != '[')
    {
        return seq;
    }

    size_t i = pos + 2;
    int currentParam = 0;
    bool hasParam = false;

    // Skip private-mode prefix characters (e.g. '?' in ESC[?7h, '>' in ESC[>c)
    // These are in the 0x3C-0x3F range per ECMA-48.
    if (i < body.size())
    {
        char prefix = body[i];
        if (prefix == '?' || prefix == '>' || prefix == '<' || prefix == '=')
        {
            ++i;
        }
    }

    while (i < body.size())
    {
        char ch = body[i];
        if (ch >= '0' && ch <= '9')
        {
            currentParam = currentParam * 10 + (ch - '0');
            hasParam = true;
            ++i;
        }
        else if (ch == ';')
        {
            seq.params.push_back(hasParam ? currentParam : 0);
            currentParam = 0;
            hasParam = false;
            ++i;
        }
        else if (ch >= 0x40 && ch <= 0x7E)
        {
            if (hasParam)
            {
                seq.params.push_back(currentParam);
            }
            seq.finalByte = ch;
            seq.length = static_cast<int>(i - pos + 1);
            return seq;
        }
        else if (ch >= 0x20 && ch <= 0x2F)
        {
            // Intermediate bytes (space, !, ", #, etc.) — skip
            ++i;
        }
        else
        {
            break; // Invalid
        }
    }
    return seq; // Incomplete
}

// ============================================================
// Render ANSI to screen buffer
// ============================================================

vector<AnsiLine> renderAnsiToScreen(const string& body, int screenWidth)
{
    VScreen screen(screenWidth);

    size_t i = 0;
    while (i < body.size())
    {
        uint8_t ch = static_cast<uint8_t>(body[i]);

        // ANSI escape sequence
        if (ch == 0x1B && i + 1 < body.size() && body[i + 1] == '[')
        {
            CsiSequence csi = parseCsi(body, i);
            if (csi.length == 0)
            {
                // Invalid/incomplete sequence — skip ESC
                ++i;
                continue;
            }

            int p0 = csi.params.empty() ? 0 : csi.params[0];
            int p1 = csi.params.size() >= 2 ? csi.params[1] : 0;

            switch (csi.finalByte)
            {
                case 'H': case 'f': // Cursor position (row;col, 1-based)
                {
                    int row = (csi.params.empty() ? 1 : std::max(1, p0)) - 1;
                    int col = (csi.params.size() < 2 ? 1 : std::max(1, p1)) - 1;
                    screen.moveCursor(row, col);
                    break;
                }
                case 'A': // Cursor up
                    screen.cursorRow = std::max(0, screen.cursorRow - std::max(1, p0));
                    screen.pendingWrap = false;
                    break;
                case 'B': // Cursor down
                    screen.cursorRow += std::max(1, p0);
                    screen.pendingWrap = false;
                    screen.ensureRow(screen.cursorRow);
                    break;
                case 'C': // Cursor forward
                    screen.cursorCol = std::min(screenWidth - 1,
                                                 screen.cursorCol + std::max(1, p0));
                    screen.pendingWrap = false;
                    break;
                case 'D': // Cursor back
                    screen.cursorCol = std::max(0, screen.cursorCol - std::max(1, p0));
                    screen.pendingWrap = false;
                    break;
                case 'E': // Cursor next line
                    screen.cursorRow += std::max(1, p0);
                    screen.cursorCol = 0;
                    screen.pendingWrap = false;
                    screen.ensureRow(screen.cursorRow);
                    break;
                case 'F': // Cursor previous line
                    screen.cursorRow = std::max(0, screen.cursorRow - std::max(1, p0));
                    screen.cursorCol = 0;
                    screen.pendingWrap = false;
                    break;
                case 'G': // Cursor horizontal absolute (1-based)
                    screen.cursorCol = std::max(0, std::min(screenWidth - 1,
                                                             std::max(1, p0) - 1));
                    screen.pendingWrap = false;
                    break;
                case 'J': // Erase in display
                    screen.eraseDisplay(p0);
                    break;
                case 'K': // Erase in line
                    screen.eraseLine(p0);
                    break;
                case 's': // Save cursor position
                    screen.savedRow = screen.cursorRow;
                    screen.savedCol = screen.cursorCol;
                    break;
                case 'u': // Restore cursor position
                    screen.cursorRow = screen.savedRow;
                    screen.cursorCol = screen.savedCol;
                    screen.pendingWrap = false;
                    screen.ensureRow(screen.cursorRow);
                    break;
                case 'm': // SGR (colors/attributes)
                    if (csi.params.empty())
                    {
                        applyAnsiSgrParam(0, screen.currentAttr);
                    }
                    else
                    {
                        // Use the extended handler for 256-color and truecolor
                        for (size_t pi = 0; pi < csi.params.size(); ++pi)
                        {
                            int p = csi.params[pi];
                            // Handle extended colors (38;5;n, 48;5;n, 38;2;r;g;b, 48;2;r;g;b)
                            if ((p == 38 || p == 48) && pi + 1 < csi.params.size())
                            {
                                if (csi.params[pi + 1] == 5 && pi + 2 < csi.params.size())
                                {
                                    // 256-color
                                    int colorIdx = csi.params[pi + 2];
                                    if (p == 38)
                                    {
                                        if (colorIdx >= 0 && colorIdx <= 7)
                                            { screen.currentAttr.fg = colorIdx; screen.currentAttr.bright = false; }
                                        else if (colorIdx >= 8 && colorIdx <= 15)
                                            { screen.currentAttr.fg = colorIdx - 8; screen.currentAttr.bright = true; }
                                        else if (colorIdx >= 232)
                                        {
                                            int gray = colorIdx - 232;
                                            if (gray < 8) { screen.currentAttr.fg = TC_BLACK; screen.currentAttr.bright = true; }
                                            else if (gray < 16) screen.currentAttr.fg = TC_WHITE;
                                            else { screen.currentAttr.fg = TC_WHITE; screen.currentAttr.bright = true; }
                                        }
                                        else
                                        {
                                            int ci = colorIdx - 16;
                                            int r = ci / 36, g = (ci / 6) % 6, b = ci % 6;
                                            if (r > g && r > b) screen.currentAttr.fg = TC_RED;
                                            else if (g > r && g > b) screen.currentAttr.fg = TC_GREEN;
                                            else if (b > r && b > g) screen.currentAttr.fg = TC_BLUE;
                                            else if (r > 0 && g > 0 && b == 0) screen.currentAttr.fg = TC_YELLOW;
                                            else if (r > 0 && b > 0 && g == 0) screen.currentAttr.fg = TC_MAGENTA;
                                            else if (g > 0 && b > 0 && r == 0) screen.currentAttr.fg = TC_CYAN;
                                            else if (r > 3 || g > 3 || b > 3) { screen.currentAttr.fg = TC_WHITE; screen.currentAttr.bright = true; }
                                            else if (r > 0 || g > 0 || b > 0) screen.currentAttr.fg = TC_WHITE;
                                            else screen.currentAttr.fg = TC_BLACK;
                                        }
                                    }
                                    else // p == 48
                                    {
                                        if (colorIdx >= 0 && colorIdx <= 7) screen.currentAttr.bg = colorIdx;
                                        else if (colorIdx >= 8 && colorIdx <= 15) screen.currentAttr.bg = colorIdx - 8;
                                        else if (colorIdx >= 232) { screen.currentAttr.bg = (colorIdx - 232 < 12) ? TC_BLACK : TC_WHITE; }
                                        else
                                        {
                                            int ci = colorIdx - 16;
                                            int r = ci / 36, g = (ci / 6) % 6, b = ci % 6;
                                            if (r > g && r > b) screen.currentAttr.bg = TC_RED;
                                            else if (g > r && g > b) screen.currentAttr.bg = TC_GREEN;
                                            else if (b > r && b > g) screen.currentAttr.bg = TC_BLUE;
                                            else if (r > 0 && g > 0) screen.currentAttr.bg = TC_YELLOW;
                                            else if (r > 0 && b > 0) screen.currentAttr.bg = TC_MAGENTA;
                                            else if (g > 0 && b > 0) screen.currentAttr.bg = TC_CYAN;
                                            else if (r > 0 || g > 0 || b > 0) screen.currentAttr.bg = TC_WHITE;
                                            else screen.currentAttr.bg = TC_BLACK;
                                        }
                                    }
                                    pi += 2;
                                }
                                else if (csi.params[pi + 1] == 2 && pi + 4 < csi.params.size())
                                {
                                    // Truecolor — map to nearest 8-color
                                    int r = csi.params[pi + 2], g = csi.params[pi + 3], b = csi.params[pi + 4];
                                    int color;
                                    if (r > g && r > b) color = TC_RED;
                                    else if (g > r && g > b) color = TC_GREEN;
                                    else if (b > r && b > g) color = TC_BLUE;
                                    else if (r > 128 && g > 128 && b < 64) color = TC_YELLOW;
                                    else if (r > 128 && b > 128 && g < 64) color = TC_MAGENTA;
                                    else if (g > 128 && b > 128 && r < 64) color = TC_CYAN;
                                    else if (r > 192 || g > 192 || b > 192) color = TC_WHITE;
                                    else if (r > 64 || g > 64 || b > 64) color = TC_WHITE;
                                    else color = TC_BLACK;
                                    if (p == 38) { screen.currentAttr.fg = color; screen.currentAttr.bright = (r > 192 || g > 192 || b > 192); }
                                    else screen.currentAttr.bg = color;
                                    pi += 4;
                                }
                                continue;
                            }
                            applyAnsiSgrParam(p, screen.currentAttr);
                        }
                    }
                    break;
                default:
                    break; // Unknown CSI — ignore
            }
            i += csi.length;
            continue;
        }

        // Regular character or control character
        screen.putChar(static_cast<char>(ch));
        ++i;
    }

    // Convert screen buffer to AnsiLine vector
    vector<AnsiLine> result;

    // Find the last non-empty row
    int lastNonEmpty = -1;
    for (int r = static_cast<int>(screen.rows.size()) - 1; r >= 0; --r)
    {
        for (const auto& cell : screen.rows[r])
        {
            if (cell.ch != ' ' || cell.attr.bg != TC_BLACK)
            {
                lastNonEmpty = r;
                break;
            }
        }
        if (lastNonEmpty >= 0) break;
    }

    for (int r = 0; r <= lastNonEmpty; ++r)
    {
        AnsiLine line;
        line.cells = screen.rows[r];
        result.push_back(std::move(line));
    }

    return result;
}
