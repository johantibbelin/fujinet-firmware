#ifndef MEATLOAF_UTILS_U8CHAR
#define MEATLOAF_UTILS_U8CHAR

#include <string>
#include <iostream>

/********************************************************
 * U8Char
 * 
 * A minimal wide char implementation that can handle UTF8
 * and convert it to PETSCII
 ********************************************************/

class U8Char {
    static const char16_t utf8map[];
    const char missing = '?';
    void fromUtf8Stream(std::istream* reader);

public:
    char16_t ch;
    U8Char(const uint16_t codepoint): ch(codepoint) {};
    U8Char(std::istream* reader) {
        fromUtf8Stream(reader);
    }
    U8Char(const char petscii) {
        ch = utf8map[(uint8_t)petscii];
    }

    size_t fromCharArray(char* reader);

    std::string toUtf8();
    uint8_t toPetscii();
    size_t toUnicode32(std::string& input_utf8, uint32_t* output_unicode32, size_t max_output_length);
    std::string fromUnicode32(uint32_t* input_unicode32, size_t input_length);
    static std::string toPunycode(std::string utf8String);
    static std::string fromPunycode(std::string punycodeString);
};

#endif /* MEATLOAF_UTILS_U8CHAR */
