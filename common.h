#pragma once

/*
 *
 * -- header with most commonly used utilites
 *
 */


//
//
// -- type definitions
//
//

#include <inttypes.h>

typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;
typedef uintptr_t   umm;

typedef int8_t      i8;
typedef int16_t     i16;
typedef int32_t     i32;
typedef int64_t     i64;
typedef intptr_t    imm;

typedef float  f32;
typedef double f64;

constexpr u32 U32_MAX = ~(u32)0;




//
//
// -- macros
//
//

//
// Defer
//

template <typename F>
struct Defer_RAII
{
    F f;
    Defer_RAII(F f): f(f) {}
    ~Defer_RAII() { f(); }
};

template <typename F>
Defer_RAII<F> defer_function(F f)
{
    return Defer_RAII<F>(f);
}


#define Concatenate__(x, y) x##y
#define Concatenate_(x, y)  Concatenate__(x, y)
#define Concatenate(x, y)   Concatenate_(x, y)


#define Defer(code)   auto Concatenate(_defer_, __COUNTER__) = defer_function([&] () { code; })


//
// Memory manipulation
//

#include <memory.h>

#define ZeroStruct(address)    memset((address), 0, sizeof(*(address)));
#define ZeroStaticArray(array) memset((array),   0, sizeof((array)));

#define ArrayCount(array) (sizeof(array) / sizeof((array)[0]))
#define ElementSize(array) sizeof((array)[0])

#define MemberSize(Type, member) sizeof(((Type*) 0)->member)




//
//
// -- Region: stack based memory utility
//
//


#include "libraries/lk_region.h"

typedef LK_Region Region;
typedef LK_Region_Cursor Region_Cursor;

#define RegionInit LK_RegionInit

#define RegionValue(region_ptr, type) LK_RegionValue(region_ptr, type) 
#define RegionArray(region_ptr, type, count) LK_RegionArray(region_ptr, type, count) 
#define RegionValueAligned(region_ptr, type, alignment) LK_RegionValueAligned(region_ptr, type, alignment) 
#define RegionArrayAligned(region_ptr, type, count, alignment) LK_RegionArrayAligned(region_ptr, type, count, alignment)


// Scoped memory
// create stack instance to set a cursor position
// at the end of the scope the cursor is rewinded

struct Scoped_Region_Cursor
{
    Region* region;
    Region_Cursor cursor;

    inline Scoped_Region_Cursor(Region* region)
    : region(region)
    {
        lk_region_cursor(region, &cursor);
    }

    inline ~Scoped_Region_Cursor()
    {
        lk_region_rewind(region, &cursor);
    }
};




//
//
// -- temporary memory. rewind each frame!
//
//

extern    Region temporary_memory;
constexpr Region* temp = &temporary_memory;




//
//
// -- lists
//
//


//
// doubly - linked intrusive list
//

template <typename T>
struct List_Link
{
    T* prev;
    T* next;
};

#define List(T, member) List_<T, &T::member>

template <typename T, List_Link<T> T::* M>
struct List_
{
    T* head;
    T* tail;
};

template <typename T, List_Link<T> T::* M>
inline void link(List_<T, M>* list, T* object)
{
    T* tail = list->tail;
    if (tail)
        (tail->*M).next = object;
    else
        list->head = object;

    (object->*M).prev = tail;
    (object->*M).next = NULL;
    list->tail = object;
}

template <typename T, List_Link<T> T::* M>
inline void unlink(List_<T, M>* list, T* object)
{
    T* prev = (object->*M).prev;
    T* next = (object->*M).next;

    if (prev)
        (prev->*M).next = next;
    else
        list->head = next;

    if (next)
        (next->*M).prev = prev;
    else
        list->tail = prev;
}

// This junk is here just so we can use the ranged for syntax on Lists.
// Ughh...

template <typename T, List_Link<T> T::* M>
struct ListIterator
{
    T* current;
    inline bool operator!=(ListIterator<T, M> other) { return current != other.current; }
    inline void operator++() { current = (current->*M).next; }
    inline void operator--() { current = (current->*M).prev; }
    inline T*   operator* () { return current; }
};

template <typename T, List_Link<T> T::* M>
inline ListIterator<T, M> begin(List_<T, M>& list) { return { list.head }; }

template <typename T, List_Link<T> T::* M>
inline ListIterator<T, M> end(List_<T, M>& list) { return { NULL }; }


//
// intrusive retirement list
//

#define RetirementLink(Type) Type* next_retired;

template <typename T>
struct Retirement_List
{
    T* head;
};

template <typename T>
void retire(Retirement_List<T>* list, T* value)
{
    value->next_retired = list->head;
    list->head = value;
}

template <typename T>
T* allocate(Region* region, Retirement_List<T>* list)
{
    T* value = list->head;
    if (value)
    {
        list->head = value->next_retired;
        ZeroStruct(value);
    }
    else
    {
        value = RegionValue(region, T);
    }

    return value;
}




//
//
// -- string utility
//
//


#include <assert.h>
#include <crtdbg.h>


#define DebugAssert(test) _ASSERT(test) //assert(test)



//
// Memory, character and C-style string utilities.
//


void copy(void* to, const void* from, umm length);
void move(void* to, void* from, umm length);
bool compare(const void* m1, const void* m2, umm length);

u16 endian_swap16(u16 value);
u32 endian_swap32(u32 value);
u64 endian_swap64(u64 value);

bool is_decimal_digit(u8 character);
bool is_whitespace(u8 character);

umm length_of_c_style_string(const char* c_string);
umm length_of_c_style_string(const u16* c_string);


//
// 8-bit strings.
// When treated as text, UTF-8 encoding is assumed.
//


struct String
{
    umm length;
    u8* data;

    inline u8& operator[](umm index)
    {
        DebugAssert(index < length);
        return data[index];
    }

    inline operator bool()
    {
        return length != 0;
    }
};

#define StringArgs(string)  (int)((string).length), (const char*)((string).data)


inline String operator ""_s(const char* c_string, umm length)
{
    String result;
    result.length = length;
    result.data = (u8*) c_string;
    return result;
}


String make_string(const char* c_string);  // Allocates.
char* make_c_style_string(String string);  // Allocates.
String wrap_string(const char* c_string);

String allocate_string(Region* memory, String string);
String clone_string(String string);  // Allocates.

String concatenate(String first, String second, String third = {}, String fourth = {}, String fifth = {}, String sixth = {});  // Allocates.
String substring(String string, umm start_index, umm length);
String concatenate_adjacent_substrings(String left, String right, String parent);

bool operator==(String lhs, String rhs);
bool operator==(String lhs, const char* rhs);
bool operator==(const char* lhs, String rhs);

inline bool operator!=(String      lhs, String      rhs) { return !(lhs == rhs); }
inline bool operator!=(String      lhs, const char* rhs) { return !(lhs == rhs); }
inline bool operator!=(const char* lhs, String      rhs) { return !(lhs == rhs); }

bool prefix_equals(String string, String prefix);
bool suffix_equals(String string, String suffix);

constexpr umm NOT_FOUND = ~(umm) 0;

umm find_first_occurance(String string, u8 of);
umm find_first_occurance(String string, String of);
umm find_first_occurance_of_any(String string, String any_of);

umm find_last_occurance(String string, u8 of);
umm find_last_occurance(String string, String of);
umm find_last_occurance_of_any(String string, String any_of);

void replace_all_occurances(String string, u8 what, u8 with_what);

u32 compute_crc32(String data);


//
// Text reading utilities.
//


void consume(String* string, umm amount);
void consume_whitespace(String* string);

String consume_line(String* string);
String consume_line_preserve_whitespace(String* string);
String peek_line_preserve_whitespace(String string); // Doesn't modify string.
String consume_until(String* string, u8 until_what);
String consume_until(String* string, String until_what);
String consume_until_any(String* string, String until_any_of);
String consume_until_whitespace(String* string);

String trim(String string);


//
// Binary reading utilities.
//


bool read_bytes(String* string, void* result, umm count);

bool read_u8 (String* string, u8*  result);
bool read_u16(String* string, u16* result);
bool read_u32(String* string, u32* result);
bool read_u64(String* string, u64* result);

bool read_i8 (String* string, i8*  result);
bool read_s16(String* string, i16* result);
bool read_s32(String* string, i32* result);
bool read_s64(String* string, i64* result);

bool read_u16le(String* string, u16* result);
bool read_u32le(String* string, u32* result);
bool read_u64le(String* string, u64* result);

bool read_u16be(String* string, u16* result);
bool read_u32be(String* string, u32* result);
bool read_u64be(String* string, u64* result);

bool read_s16le(String* string, i16* result);
bool read_s32le(String* string, i32* result);
bool read_s64le(String* string, i64* result);

bool read_s16be(String* string, i16* result);
bool read_s32be(String* string, i32* result);
bool read_s64be(String* string, i64* result);


//
// File path utilities.
// Convention: directory paths *don't* end with a trailing slash.
//


String get_file_name(String path);
String get_file_name_without_extension(String path);

String get_parent_directory_path(String path);


//
// 16-bit strings. These mostly exist for interfacing with Windows.
// That's why conversion routines return null terminated strings, and why
// we don't really support any operations on them.
//


struct String16
{
    umm length;
    u16* data;

    inline u16& operator[](umm index)
    {
        DebugAssert(index < length);
        return data[index];
    }

    inline operator bool()
    {
        return length != 0;
    }
};

String16 make_string16(const u16* c_string);    // Allocates. The returned string is null terminated.
String16 convert_utf8_to_utf16(String string);  // Allocates. The returned string is null terminated.
String convert_utf16_to_utf8(String16 string);  // Allocates. The returned string is null terminated.


//
// String building utilities.
//


struct String_Builder
{
    String string;  // Heap allocated and null terminated.
    umm capacity;
};


void free_string_builder(String_Builder* builder);
void clear(String_Builder* builder);
void append(String_Builder* builder, const void* data, umm length);
void insert(String_Builder* builder, umm at_offset, const void* data, umm length);
void remove(String_Builder* builder, umm at_offset, umm length);


inline void append(String_Builder* builder, String string)
{
    append(builder, string.data, string.length);
}

inline void append(String_Builder* builder, const char* c_string)
{
    append(builder, c_string, length_of_c_style_string(c_string));
}


inline void insert(String_Builder* builder, umm at_offset, String string)
{
    insert(builder, at_offset, string.data, string.length);
}

inline void insert(String_Builder* builder, umm at_offset, const char* c_string)
{
    insert(builder, at_offset, c_string, length_of_c_style_string(c_string));
}