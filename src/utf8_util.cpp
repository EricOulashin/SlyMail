#include "utf8_util.h"
#include <cstdint>

using std::string;

// ============================================================
// UTF-8 validation and traversal
// ============================================================

// Return the expected byte length of a UTF-8 character from its lead byte
static int utf8CharLen(uint8_t leadByte)
{
    if (leadByte < 0x80) return 1;
    if ((leadByte & 0xE0) == 0xC0) return 2;
    if ((leadByte & 0xF0) == 0xE0) return 3;
    if ((leadByte & 0xF8) == 0xF0) return 4;
    return 1; // Invalid lead byte, treat as single byte
}

static bool isUtf8Continuation(uint8_t byte)
{
    return (byte & 0xC0) == 0x80;
}

bool isValidUtf8(const string& s)
{
    size_t i = 0;
    while (i < s.size())
    {
        uint8_t b = static_cast<uint8_t>(s[i]);
        int len = utf8CharLen(b);

        if (i + len > s.size())
        {
            return false;
        }

        for (int j = 1; j < len; ++j)
        {
            if (!isUtf8Continuation(static_cast<uint8_t>(s[i + j])))
            {
                return false;
            }
        }
        i += len;
    }
    return true;
}

size_t utf8NextChar(const string& s, size_t pos)
{
    if (pos >= s.size()) return s.size();
    int len = utf8CharLen(static_cast<uint8_t>(s[pos]));
    size_t next = pos + len;
    return (next > s.size()) ? s.size() : next;
}

size_t utf8PrevChar(const string& s, size_t pos)
{
    if (pos == 0) return 0;
    --pos;
    while (pos > 0 && isUtf8Continuation(static_cast<uint8_t>(s[pos])))
    {
        --pos;
    }
    return pos;
}

// Decode a UTF-8 character at position pos, return the Unicode codepoint.
// Advances pos past the character.
static uint32_t utf8Decode(const string& s, size_t& pos)
{
    if (pos >= s.size()) return 0;

    uint8_t b0 = static_cast<uint8_t>(s[pos]);

    if (b0 < 0x80)
    {
        ++pos;
        return b0;
    }

    uint32_t cp = 0;
    int len = utf8CharLen(b0);

    switch (len)
    {
        case 2:
            cp = b0 & 0x1F;
            break;
        case 3:
            cp = b0 & 0x0F;
            break;
        case 4:
            cp = b0 & 0x07;
            break;
        default:
            ++pos;
            return b0; // Invalid
    }

    for (int i = 1; i < len; ++i)
    {
        if (pos + i >= s.size()) { pos = s.size(); return cp; }
        cp = (cp << 6) | (static_cast<uint8_t>(s[pos + i]) & 0x3F);
    }

    pos += len;
    return cp;
}

// ============================================================
// Display width calculation
// ============================================================

// Simple CJK/fullwidth detection.
// Returns 2 for fullwidth characters, 1 for others, 0 for control.
static int charDisplayWidth(uint32_t cp)
{
    // Control characters
    if (cp < 0x20 || cp == 0x7F) return 0;

    // CJK Unified Ideographs
    if (cp >= 0x4E00 && cp <= 0x9FFF) return 2;
    // CJK Unified Ideographs Extension A
    if (cp >= 0x3400 && cp <= 0x4DBF) return 2;
    // CJK Compatibility Ideographs
    if (cp >= 0xF900 && cp <= 0xFAFF) return 2;
    // Fullwidth forms
    if (cp >= 0xFF01 && cp <= 0xFF60) return 2;
    if (cp >= 0xFFE0 && cp <= 0xFFE6) return 2;
    // Hangul syllables
    if (cp >= 0xAC00 && cp <= 0xD7AF) return 2;
    // CJK Extension B and beyond
    if (cp >= 0x20000 && cp <= 0x2FA1F) return 2;

    return 1;
}

int utf8DisplayWidth(const string& s)
{
    int width = 0;
    size_t pos = 0;
    while (pos < s.size())
    {
        uint32_t cp = utf8Decode(s, pos);
        width += charDisplayWidth(cp);
    }
    return width;
}

size_t utf8ColToByteOffset(const string& s, int col)
{
    int currentCol = 0;
    size_t pos = 0;
    while (pos < s.size() && currentCol < col)
    {
        size_t charStart = pos;
        uint32_t cp = utf8Decode(s, pos);
        int w = charDisplayWidth(cp);
        if (currentCol + w > col)
        {
            return charStart; // Would overshoot
        }
        currentCol += w;
    }
    if (currentCol <= col && pos <= s.size())
    {
        return pos;
    }
    return string::npos;
}

int utf8ByteOffsetToCol(const string& s, size_t offset)
{
    int col = 0;
    size_t pos = 0;
    while (pos < s.size() && pos < offset)
    {
        uint32_t cp = utf8Decode(s, pos);
        col += charDisplayWidth(cp);
    }
    return col;
}

string utf8Truncate(const string& s, int maxCols)
{
    int width = 0;
    size_t pos = 0;

    while (pos < s.size())
    {
        size_t prevPos = pos;
        uint32_t cp = utf8Decode(s, pos);
        int w = charDisplayWidth(cp);
        if (width + w > maxCols)
        {
            return s.substr(0, prevPos);
        }
        width += w;
    }
    return s;
}

// ============================================================
// CP437 to UTF-8 conversion
// ============================================================

// CP437 to Unicode codepoint mapping for bytes 0x00-0xFF
// Bytes 0x00-0x1F are control characters in ASCII but have graphic
// representations in CP437. Bytes 0x20-0x7E are standard ASCII.
// Bytes 0x7F-0xFF have special CP437 characters.
static const uint32_t cp437ToUnicode[256] = {
    // 0x00-0x0F: CP437 graphical characters (traditionally displayed)
    0x0000, 0x263A, 0x263B, 0x2665, 0x2666, 0x2663, 0x2660, 0x2022,
    0x25D8, 0x25CB, 0x25D9, 0x2642, 0x2640, 0x266A, 0x266B, 0x263C,
    // 0x10-0x1F
    0x25BA, 0x25C4, 0x2195, 0x203C, 0x00B6, 0x00A7, 0x25AC, 0x21A8,
    0x2191, 0x2193, 0x2192, 0x2190, 0x221F, 0x2194, 0x25B2, 0x25BC,
    // 0x20-0x7E: Standard ASCII (same codepoints)
    0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
    0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
    0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
    0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
    0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
    0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
    0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
    0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
    0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
    0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
    0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E,
    // 0x7F
    0x2302,
    // 0x80-0x8F: Latin accented characters
    0x00C7, 0x00FC, 0x00E9, 0x00E2, 0x00E4, 0x00E0, 0x00E5, 0x00E7,
    0x00EA, 0x00EB, 0x00E8, 0x00EF, 0x00EE, 0x00EC, 0x00C4, 0x00C5,
    // 0x90-0x9F
    0x00C9, 0x00E6, 0x00C6, 0x00F4, 0x00F6, 0x00F2, 0x00FB, 0x00F9,
    0x00FF, 0x00D6, 0x00DC, 0x00A2, 0x00A3, 0x00A5, 0x20A7, 0x0192,
    // 0xA0-0xAF
    0x00E1, 0x00ED, 0x00F3, 0x00FA, 0x00F1, 0x00D1, 0x00AA, 0x00BA,
    0x00BF, 0x2310, 0x00AC, 0x00BD, 0x00BC, 0x00A1, 0x00AB, 0x00BB,
    // 0xB0-0xBF: Box drawing (light shade, medium shade, dark shade, then lines)
    0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556,
    0x2555, 0x2563, 0x2551, 0x2557, 0x255D, 0x255C, 0x255B, 0x2510,
    // 0xC0-0xCF
    0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x255E, 0x255F,
    0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x2567,
    // 0xD0-0xDF
    0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256B,
    0x256A, 0x2518, 0x250C, 0x2588, 0x2584, 0x258C, 0x2590, 0x2580,
    // 0xE0-0xEF: Greek and math symbols
    0x03B1, 0x00DF, 0x0393, 0x03C0, 0x03A3, 0x03C3, 0x00B5, 0x03C4,
    0x03A6, 0x0398, 0x03A9, 0x03B4, 0x221E, 0x03C6, 0x03B5, 0x2229,
    // 0xF0-0xFF
    0x2261, 0x00B1, 0x2265, 0x2264, 0x2320, 0x2321, 0x00F7, 0x2248,
    0x00B0, 0x2219, 0x00B7, 0x221A, 0x207F, 0x00B2, 0x25A0, 0x00A0,
};

// Encode a Unicode codepoint as UTF-8
static string encodeUtf8(uint32_t cp)
{
    string result;
    if (cp < 0x80)
    {
        result += static_cast<char>(cp);
    }
    else if (cp < 0x800)
    {
        result += static_cast<char>(0xC0 | (cp >> 6));
        result += static_cast<char>(0x80 | (cp & 0x3F));
    }
    else if (cp < 0x10000)
    {
        result += static_cast<char>(0xE0 | (cp >> 12));
        result += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        result += static_cast<char>(0x80 | (cp & 0x3F));
    }
    else if (cp < 0x110000)
    {
        result += static_cast<char>(0xF0 | (cp >> 18));
        result += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
        result += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        result += static_cast<char>(0x80 | (cp & 0x3F));
    }
    return result;
}

string cp437CharToUtf8(uint8_t ch)
{
    uint32_t cp = cp437ToUnicode[ch];
    if (cp == 0 && ch == 0)
    {
        return ""; // Null byte
    }
    return encodeUtf8(cp);
}

string cp437ToUtf8(const string& cp437text)
{
    string result;
    result.reserve(cp437text.size() * 2); // UTF-8 may be longer
    for (size_t i = 0; i < cp437text.size(); ++i)
    {
        uint8_t ch = static_cast<uint8_t>(cp437text[i]);
        // Pass through newlines and tabs
        if (ch == '\n' || ch == '\r' || ch == '\t')
        {
            result += static_cast<char>(ch);
            continue;
        }
        // Standard printable ASCII passes through
        if (ch >= 0x20 && ch <= 0x7E)
        {
            result += static_cast<char>(ch);
            continue;
        }
        // Convert everything else via the table
        result += cp437CharToUtf8(ch);
    }
    return result;
}
