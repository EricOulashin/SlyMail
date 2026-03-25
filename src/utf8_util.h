#ifndef SLYMAIL_UTF8_UTIL_H
#define SLYMAIL_UTF8_UTIL_H

// UTF-8 Utility Functions
//
// Provides helpers for working with UTF-8 encoded strings in a
// terminal environment where display width matters.

#include <string>
#include <cstdint>

// Check if a string is valid UTF-8
bool isValidUtf8(const std::string& s);

// Get the display width of a UTF-8 string (number of terminal columns).
// Accounts for multi-byte characters. Double-width CJK characters
// count as 2 columns. Control characters count as 0.
int utf8DisplayWidth(const std::string& s);

// Get the byte offset of the Nth display column in a UTF-8 string.
// Returns string::npos if col is beyond the end of the string.
size_t utf8ColToByteOffset(const std::string& s, int col);

// Get the display column for a given byte offset in a UTF-8 string.
int utf8ByteOffsetToCol(const std::string& s, size_t offset);

// Advance past one UTF-8 character from the given byte position.
// Returns the byte position after the character.
size_t utf8NextChar(const std::string& s, size_t pos);

// Move back one UTF-8 character from the given byte position.
// Returns the byte position of the previous character start.
size_t utf8PrevChar(const std::string& s, size_t pos);

// Truncate a UTF-8 string to fit within maxCols display columns.
// Does not split multi-byte characters.
std::string utf8Truncate(const std::string& s, int maxCols);

// Convert a CP437-encoded string to UTF-8.
// Maps each byte 0x00-0xFF to its Unicode equivalent.
std::string cp437ToUtf8(const std::string& cp437text);

// Convert a single CP437 byte to its UTF-8 character string.
std::string cp437CharToUtf8(uint8_t ch);

#endif // SLYMAIL_UTF8_UTIL_H
