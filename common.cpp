#include "common.h"

#include <stdlib.h>


/*
 *
 * -- implementation of common.h subitems that need .cpp implementation
 *
 */


//
//
// -- temporary memory
//
//

Region temporary_memory = {};




//
//
// -- string utility
//
//

//
// Memory, character and C-style string utilities.
//


void copy(void* to, const void* from, umm length)
{
    u8* to_bytes   = (u8*) to;
    u8* from_bytes = (u8*) from;
    for (umm i = 0; i < length; i++)
        *(to_bytes++) = *(from_bytes++);
}

void move(void* to, void* from, umm length)
{
    if (to < from)
    {
        u8* to_bytes   = (u8*) to;
        u8* from_bytes = (u8*) from;
        for (umm i = 0; i < length; i++)
            *(to_bytes++) = *(from_bytes++);
    }
    else
    {
        u8* to_bytes   = (u8*) to   + length;
        u8* from_bytes = (u8*) from + length;
        for (umm i = 0; i < length; i++)
            *(--to_bytes) = *(--from_bytes);
    }
}

bool compare(const void* m1, const void* m2, umm length)
{
    const u8* bytes1 = (const u8*) m1;
    const u8* bytes2 = (const u8*) m2;

    for (umm i = 0; i < length; i++)
    {
        if (*bytes1 != *bytes2)
            return false;

        bytes1++;
        bytes2++;
    }

    return true;
}


u16 endian_swap16(u16 value)
{
    return (value << 8) | (value >> 8);
}

u32 endian_swap32(u32 value)
{
    return (value << 24) | (value >> 24) |
           ((value << 8) & 0xff0000ul) |
           ((value >> 8) & 0x00ff00ul);
}

u64 endian_swap64(u64 value)
{
    u32 low32  = (u32) value;
    u32 high32 = (u32)(value >> 32);

    u32 new_low32  = endian_swap32(high32);
    u32 new_high32 = endian_swap32(low32);

    return (u64) new_low32 | ((u64) new_high32 << 32);
}


static String line_ending_chars = "\n\r"_s;
static String whitespace_chars = " \t\n\r"_s;
static String slash_chars = "/\\"_s;

bool is_decimal_digit(u8 character)
{
    return character >= '0' && character <= '9';
}

bool is_whitespace(u8 character)
{
    if (character == ' ')  return true;
    if (character == '\t') return true;
    if (character == '\n') return true;
    if (character == '\r') return true;

    return false;
}


umm length_of_c_style_string(const char* c_string)
{
    umm length = 0;
    while (c_string[length])
        length++;
    return length;
}

umm length_of_c_style_string(const u16* c_string)
{
    umm length = 0;
    while (c_string[length])
        length++;
    return length;
}


//
// 8-bit strings.
// When treated as text, UTF-8 encoding is assumed.
//


String make_string(const char* c_string)
{
    umm length = length_of_c_style_string(c_string);

    String result;
    result.length = length;
    result.data = LK_RegionArray(temp, u8, length);

    copy(result.data, c_string, length);

    return result;
}

char* make_c_style_string(String string)
{
    char* result = LK_RegionArray(temp, char, string.length + 1);

    copy(result, string.data, string.length);
    result[string.length] = 0;

    return result;
}

String wrap_string(const char* c_string)
{
    String result;
    result.length = length_of_c_style_string(c_string);
    result.data = (u8*) c_string;
    return result;
}


String allocate_string(Region* memory, String string)
{
    String result;
    result.length = string.length;
    result.data = LK_RegionArray(memory, u8, string.length);

    copy(result.data, string.data, string.length);

    return result;
}


String clone_string(String string)
{
    return allocate_string(temp, string);
}


String concatenate(String first, String second, String third, String fourth, String fifth, String sixth)
{
    String result;
    result.length = first.length + second.length + third.length + fourth.length + fifth.length + sixth.length;
    result.data = LK_RegionArray(temp, u8, result.length);

    u8* write = result.data;

    copy(write, first.data, first.length);
    write += first.length;

    copy(write, second.data, second.length);
    write += second.length;

    copy(write, third.data, third.length);
    write += third.length;

    copy(write, fourth.data, fourth.length);
    write += fourth.length;

    copy(write, fifth.data, fifth.length);
    write += fifth.length;

    copy(write, sixth.data, sixth.length);
    write += sixth.length;

    return result;
}


String substring(String string, umm start_index, umm length)
{
    DebugAssert(start_index <= string.length);
    DebugAssert((start_index + length) <= string.length);

    String result;
    result.length = length;
    result.data = string.data + start_index;

    return result;
}

// concatenates everything from left[0] to right[end]
String concatenate_adjacent_substrings(String left, String right, String parent)
{
    if (left.data == NULL && left.length == 0) // left string can be empty
        return right;

    DebugAssert(left.data >= parent.data);
    DebugAssert(right.data + right.length <= parent.data + parent.length);

    String result;
    result.data = left.data;
    result.length = left.length + right.length;

    return result;
}


bool operator==(String lhs, String rhs)
{
    if (lhs.length != rhs.length)
        return false;
    return compare(lhs.data, rhs.data, lhs.length);
}

bool operator==(String lhs, const char* rhs)
{
    umm rhs_length = length_of_c_style_string(rhs);
    if (lhs.length != rhs_length)
        return false;
    return compare(lhs.data, rhs, lhs.length);
}

bool operator==(const char* lhs, String rhs)
{
    umm lhs_length = length_of_c_style_string(lhs);
    if (lhs_length != rhs.length)
        return false;
    return compare(lhs, rhs.data, lhs_length);
}


bool prefix_equals(String string, String prefix)
{
    if (string.length < prefix.length)
        return false;

    return compare(string.data, prefix.data, prefix.length);
}

bool suffix_equals(String string, String suffix)
{
    if (string.length < suffix.length)
        return false;

    u8* substring = string.data + string.length - suffix.length;
    return compare(substring, suffix.data, suffix.length);
}


umm find_first_occurance(String string, u8 of)
{
    for (imm i = 0; i < string.length; i++)
        if (string[i] == of)
            return i;

    return NOT_FOUND;
}

umm find_first_occurance(String string, String of)
{
    if (string.length < of.length)
        return NOT_FOUND;

    for (imm i = 0; i <= string.length - of.length; i++)
        if (compare(string.data + i, of.data, of.length))
            return i;

    return NOT_FOUND;
}

umm find_first_occurance_of_any(String string, String any_of)
{
    for (imm i = 0; i < string.length; i++)
    {
        u8 c = string[i];

        for (umm j = 0; j < any_of.length; j++)
        {
            u8 c2 = any_of[j];
            if (c == c2)
                return i;
        }
    }

    return NOT_FOUND;
}


umm find_last_occurance(String string, u8 of)
{
    for (imm i = string.length - 1; i >= 0; i--)
        if (string[i] == of)
            return i;

    return NOT_FOUND;
}

umm find_last_occurance(String string, String of)
{
    if (string.length < of.length)
        return NOT_FOUND;

    for (imm i = string.length - of.length; i >= 0; i--)
        if (compare(string.data + i, of.data, of.length))
            return i;

    return NOT_FOUND;
}

umm find_last_occurance_of_any(String string, String any_of)
{
    for (imm i = string.length - 1; i >= 0; i--)
    {
        u8 c = string[i];

        for (umm j = 0; j < any_of.length; j++)
        {
            u8 c2 = any_of[j];
            if (c == c2)
                return i;
        }
    }

    return NOT_FOUND;
}


void replace_all_occurances(String string, u8 what, u8 with_what)
{
    for (imm i = string.length - 1; i >= 0; i--)
        if (string[i] == what)
            string[i] = with_what;
}


//u32 compute_crc32(String data)
//{
//    u32 crc = U32_MAX;
//
//    for (umm i = 0; i < data.length; i++)
//    {
//        crc = crc ^ data.data[i];
//        crc = (crc >> 1) ^ (0xedb88320 & -(crc & 1));
//        crc = (crc >> 1) ^ (0xedb88320 & -(crc & 1));
//        crc = (crc >> 1) ^ (0xedb88320 & -(crc & 1));
//        crc = (crc >> 1) ^ (0xedb88320 & -(crc & 1));
//        crc = (crc >> 1) ^ (0xedb88320 & -(crc & 1));
//        crc = (crc >> 1) ^ (0xedb88320 & -(crc & 1));
//        crc = (crc >> 1) ^ (0xedb88320 & -(crc & 1));
//        crc = (crc >> 1) ^ (0xedb88320 & -(crc & 1));
//    }
//
//    return ~crc;
//}


//
//
// Text reading utilities.
//
//


void consume(String* string, umm amount)
{
    DebugAssert(amount <= string->length);
    string->data += amount;
    string->length -= amount;
}

void consume_whitespace(String* string)
{
    while (*string && is_whitespace(string->data[0]))
        consume(string, 1);
}

String consume_line(String* string)
{
    consume_whitespace(string);

    umm line_length = find_first_occurance_of_any(*string, line_ending_chars);
    if (line_length == NOT_FOUND)
        line_length = string->length;

    String line = substring(*string, 0, line_length);
    consume(string, line_length);

    return line;
}

String consume_line_preserve_whitespace(String* string)
{
    umm line_length = find_first_occurance_of_any(*string, "\n\r"_s);
    if (line_length == NOT_FOUND)
        line_length = string->length;

    String line = substring(*string, 0, line_length);
    consume(string, line_length);

    // If we've found the line ending, consume it.
    if (*string)
    {
        umm ending_length = 1;
        if (string->length > 1)
        {
            // Handle two-u8 line endings.
            u8 c1 = string->data[0];
            u8 c2 = string->data[1];
            if ((c1 == '\n' && c2 == '\r') ||
                (c1 == '\r' && c2 == '\n'))
                ending_length++;
        }
        consume(string, ending_length);
    }

    return line;
}

String peek_line_preserve_whitespace(String string)
{
    umm line_length = find_first_occurance_of_any(string, "\n\r"_s);
    if (line_length == NOT_FOUND)
        line_length = string.length;

    String line = substring(string, 0, line_length);
    consume(&string, line_length);

    // If we've found the line ending, consume it.
    if (string)
    {
        umm ending_length = 1;
        if (string.length > 1)
        {
            // Handle two-u8 line endings.
            u8 c1 = string.data[0];
            u8 c2 = string.data[1];
            if ((c1 == '\n' && c2 == '\r') ||
                (c1 == '\r' && c2 == '\n'))
                ending_length++;
        }
        consume(&string, ending_length);
    }

    return line;
}

String consume_until(String* string, u8 until_what)
{
    consume_whitespace(string);

    umm left_length = find_first_occurance(*string, until_what);
    if (left_length == NOT_FOUND)
        left_length = string->length;

    String left = substring(*string, 0, left_length);
    consume(string, left_length);

    // If we've found the delimiter, consume it.
    if (*string)
        consume(string, 1);

    return left;
}

String consume_until(String* string, String until_what)
{
    consume_whitespace(string);

    umm left_length = find_first_occurance(*string, until_what);
    if (left_length == NOT_FOUND)
        left_length = string->length;

    String left = substring(*string, 0, left_length);
    consume(string, left_length);

    // If we've found the delimiter, consume it.
    if (*string)
        consume(string, until_what.length);

    return left;
}

String consume_until_any(String* string, String until_what)
{
    consume_whitespace(string);

    umm left_length = find_first_occurance_of_any(*string, until_what);
    if (left_length == NOT_FOUND)
        left_length = string->length;

    String left = substring(*string, 0, left_length);
    consume(string, left_length);

    // If we've found the delimiter, consume it.
    if (*string)
        consume(string, 1);

    return left;
}

String consume_until_whitespace(String* string)
{
    return consume_until_any(string, whitespace_chars);
}


String trim(String string)
{
    while (string && is_whitespace(string[0]))
        consume(&string, 1);
    while (string && is_whitespace(string[string.length - 1]))
        string.length--;
    return string;
}


//
//
// Binary reading utilities.
//
//


bool read_bytes(String* string, void* result, umm count)
{
    if (string->length < count)
        return false;

    copy(result, string->data, count);
    consume(string, count);
    return true;
}


bool read_u8 (String* string, u8*  result) { return read_bytes(string, result, 1); }
bool read_u16(String* string, u16* result) { return read_bytes(string, result, 2); }
bool read_u32(String* string, u32* result) { return read_bytes(string, result, 4); }
bool read_u64(String* string, u64* result) { return read_bytes(string, result, 8); }

bool read_i8 (String* string, i8*  result) { return read_bytes(string, result, 1); }
bool read_i16(String* string, i16* result) { return read_bytes(string, result, 2); }
bool read_i32(String* string, i32* result) { return read_bytes(string, result, 4); }
bool read_i64(String* string, i64* result) { return read_bytes(string, result, 8); }


#ifdef BIG_ENDIAN
#error "Go away, we don't like you!"
#endif

bool read_u16le(String* string, u16* result) { return read_bytes(string, result, 2); }
bool read_u32le(String* string, u32* result) { return read_bytes(string, result, 4); }
bool read_u64le(String* string, u64* result) { return read_bytes(string, result, 8); }

bool read_u16be(String* string, u16* result) { Defer(*result = endian_swap16(*result)); return read_bytes(string, result, 2); }
bool read_u32be(String* string, u32* result) { Defer(*result = endian_swap32(*result)); return read_bytes(string, result, 4); }
bool read_u64be(String* string, u64* result) { Defer(*result = endian_swap64(*result)); return read_bytes(string, result, 8); }

bool read_i16le(String* string, i16* result) { return read_bytes(string, result, 2); }
bool read_i32le(String* string, i32* result) { return read_bytes(string, result, 4); }
bool read_i64le(String* string, i64* result) { return read_bytes(string, result, 8); }

bool read_i16be(String* string, i16* result) { Defer(*result = (i16) endian_swap16((u16) *result)); return read_bytes(string, result, 2); }
bool read_i32be(String* string, i32* result) { Defer(*result = (i32) endian_swap32((u32) *result)); return read_bytes(string, result, 4); }
bool read_i64be(String* string, i64* result) { Defer(*result = (i64) endian_swap64((u64) *result)); return read_bytes(string, result, 8); }


//
// File path utilities.
//


String get_file_name(String path)
{
    umm last_slash_index = find_last_occurance_of_any(path, slash_chars);
    if (last_slash_index == NOT_FOUND)
        last_slash_index = 0;

    umm start_index = last_slash_index + 1;
    umm length = path.length - start_index;

    String file_name = substring(path, start_index, length);
    return file_name;
}


String get_file_name_without_extension(String path)
{
    umm last_slash_index = find_last_occurance_of_any(path, slash_chars);
    if (last_slash_index != NOT_FOUND)
        consume(&path, last_slash_index + 1);

    umm last_dot_index = find_last_occurance(path, '.');
    if (last_dot_index != NOT_FOUND)
        path.length = last_dot_index;

    return path;
}


String get_parent_directory_path(String path)
{
    umm last_slash_index = find_last_occurance_of_any(path, slash_chars);
    // @Incomplete if (last_slash_index == NOT_FOUND)

    String parent = substring(path, 0, last_slash_index);
    return parent;
}



//
// 16-bit strings. These mostly exist for interfacing with Windows.
// That's why conversion routines return null terminated strings, and why
// we don't really support any operations on them.
//



static inline bool is_legal_code_point(u32 code_point)
{
    if (code_point > 0x10FFFF) return false;
    if (code_point >= 0xD800 && code_point <= 0xDFFF) return false;
    return true;
}


static inline u32 get_utf8_sequence_length(u32 code_point)
{
    if (code_point <      0x80) return 1;
    if (code_point <     0x800) return 2;
    if (code_point <   0x10000) return 3;
    if (code_point <  0x200000) return 4;
    if (code_point < 0x4000000) return 5;
                                return 6;
}


static void encode_utf8_sequence(u32 code_point, u8* target, u32 length)
{
    switch (length)
    {
    case 6:  target[5] = (u8)((code_point | 0x80) & 0xBF); code_point >>= 6;  // fall-through
    case 5:  target[4] = (u8)((code_point | 0x80) & 0xBF); code_point >>= 6;  // fall-through
    case 4:  target[3] = (u8)((code_point | 0x80) & 0xBF); code_point >>= 6;  // fall-through
    case 3:  target[2] = (u8)((code_point | 0x80) & 0xBF); code_point >>= 6;  // fall-through
    case 2:  target[1] = (u8)((code_point | 0x80) & 0xBF); code_point >>= 6;  // fall-through
    }

    static constexpr u8 FIRST_MASK[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };
    target[0] = (u8)(code_point | FIRST_MASK[length]);
}


static bool decode_utf8_sequence(String* string, u32* out_code_point)
{
    umm length = string->length;
    u8* data = string->data;
    DebugAssert(length);

    u8 unit1 = data[0];
    consume(string, 1);

    if (unit1 < 0x80)
    {
        *out_code_point = unit1;
        return true;
    }

    if (unit1 >= 0x80 && unit1 <= 0xBF)
        return false;


    u32 consume_count = 0;
    u32 code_point = unit1;

    #define HandleUnit                                  \
    {                                                   \
        if (!--length)  return false;                   \
        u8 unit = data[++consume_count];                \
        if (unit < 0x80 || unit > 0xBF)  return false;  \
        code_point = (code_point << 6) + unit;          \
    }

    if (unit1 >= 0xFC) HandleUnit  // 6 units
    if (unit1 >= 0xF8) HandleUnit  // 5 units
    if (unit1 >= 0xF0) HandleUnit  // 4 units
    if (unit1 >= 0xE0) HandleUnit  // 3 units
    HandleUnit                     // 2 units

    #undef HandleUnit


    static constexpr u32 DECODING_MAGIC[] =
    {
        0x00000000,
        0x00000000, 0x00003080, 0x000E2080,
        0x03C82080, 0xFA082080, 0x82082080
    };

    code_point -= DECODING_MAGIC[consume_count + 1];
    consume(string, consume_count);

    *out_code_point = code_point;
    return true;
}


static void encode_utf16_sequence(u32 code_point, u16* target, u32 length)
{
    if (length == 2)
    {
        code_point -= 0x10000;
        target[0] = 0xD800 + (u16)(code_point >> 10);
        target[1] = 0xDC00 + (u16)(code_point & 0x3FF);
        return;
    }

    target[0] = (u16) code_point;
}


// If this function finds an unpaired surrogate, it will decode it as a
// code point equal to that surrogate. It is up to the caller to check if
// the returned value is a legal Unicode code point, if relevant.
static u32 decode_utf16_sequence(String16* string)
{
    DebugAssert(string->length);
    u32 unit1 = string->data[0];
    string->data++;
    string->length--;

    if (unit1 >= 0xD800 && unit1 <= 0xDBFF)
    {
        if (!string->length)
            return unit1;  // Unpaired high surrogate.

        u32 unit2 = string->data[0];
        if (unit2 < 0xDC00 || unit2 > 0xDFFF)
            return unit1;  // Unpaired high surrogate.

        string->data++;
        string->length--;
        return (((unit1 - 0xD800) << 10) | (unit2 - 0xDC00)) + 0x10000;
    }

    // Might be an unpaired low surrogate, but it makes no difference.
    return unit1;
}


static umm convert_utf8_to_utf16(String16* target, String source)
{
    umm utf16_length = 0;

    while (source)
    {
        u32 code_point;
        if (!decode_utf8_sequence(&source, &code_point))
            continue;

        u32 sequence_length = (code_point < 0x10000) ? 1 : 2;
        if (target)
        {
            u16* sequence = target->data + utf16_length;
            encode_utf16_sequence(code_point, sequence, sequence_length);
        }

        utf16_length += sequence_length;
    }

    return utf16_length;
}


static umm convert_utf16_to_utf8(String* target, String16 source)
{
    umm utf8_length = 0;

    while (source)
    {
        u32 code_point = decode_utf16_sequence(&source);

        u32 sequence_length = get_utf8_sequence_length(code_point);
        if (target)
        {
            u8* sequence = target->data + utf8_length;
            encode_utf8_sequence(code_point, sequence, sequence_length);
        }

        utf8_length += sequence_length;
    }

    return utf8_length;
}


// The returned string is null terminated.
String16 make_string16(const u16* c_string)
{
    umm length = length_of_c_style_string(c_string);

    String16 result;
    result.length = length;
    result.data = LK_RegionArray(temp, u16, length + 1);

    copy(result.data, c_string, 2 * (length + 1));

    return result;
}

// The returned string is null terminated.
String16 convert_utf8_to_utf16(String string)
{
    umm length = convert_utf8_to_utf16(NULL, string);

    String16 string16;
    string16.length = length;
    string16.data = LK_RegionArray(temp, u16, length + 1);
    string16.data[length] = 0;

    length = convert_utf8_to_utf16(&string16, string);
    DebugAssert(string16.length == length);

    return string16;
}

// The returned string is null terminated.
String convert_utf16_to_utf8(String16 string)
{
    umm length = convert_utf16_to_utf8(NULL, string);

    String string8;
    string8.length = length;
    string8.data = LK_RegionArray(temp, u8, length + 1);
    string8.data[length] = 0;

    length = convert_utf16_to_utf8(&string8, string);
    DebugAssert(string8.length == length);

    return string8;
}



//
// String building utilities.
//



void free_string_builder(String_Builder* builder)
{
    free(builder->string.data);
    ZeroStruct(builder);
}


void clear(String_Builder* builder)
{
    builder->string.length = 0;
}


void maybe_resize(String_Builder* builder, umm insert_length)
{
    umm new_length = builder->string.length + insert_length;
    if (new_length >= builder->capacity)
    {
        umm new_capacity = builder->capacity;
        if (!new_capacity) new_capacity = 64;

        // growth factor of 1.5
        do new_capacity = new_capacity + (new_capacity >> 1);
        while (new_length >= new_capacity);

        u8* new_string = (u8*) malloc(new_capacity);
        if (builder->string.data)
        {
            copy(new_string, builder->string.data, builder->string.length);
            free(builder->string.data);
        }

        builder->string.data = new_string;
        builder->capacity = new_capacity;
    }
}


void append(String_Builder* builder, const void* data, umm length)
{
    maybe_resize(builder, length);
    copy(builder->string.data + builder->string.length, data, length);
    builder->string.length += length;
    builder->string.data[builder->string.length] = 0;
}


void insert(String_Builder* builder, umm at_offset, const void* data, umm length)
{
    maybe_resize(builder, length);

    umm string_length = builder->string.length;
    DebugAssert(at_offset <= string_length);
    builder->string.length += length;

    u8* from = builder->string.data + at_offset;
    u8* to = from + length;
    umm move_size = string_length - at_offset + 1;

    move(to, from, move_size);
    copy(from, data, length);
}


void remove(String_Builder* builder, umm at_offset, umm length)
{
    umm string_length = builder->string.length;
    DebugAssert(at_offset <= string_length);
    DebugAssert(at_offset + length <= string_length);
    builder->string.length -= length;

    u8* to = builder->string.data + at_offset;
    u8* from = to + length;
    umm move_size = string_length - at_offset - length + 1;
    move(to, from, move_size);
}