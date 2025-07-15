#include "utils.h"

wide_string_t to_wide_string(const char16_t* string) {
    // ����������� �������� ������ � wide_string (������������ UTF-16LE)
    return std::basic_string<wide_char_t>(reinterpret_cast<const wide_char_t*>(string));
}