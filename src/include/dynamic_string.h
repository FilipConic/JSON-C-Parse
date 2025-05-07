#ifndef DYNAMIC_STRING_H
#define DYNAMIC_STRING_H

#include "arena.h"

typedef struct {
	char* buffer;
	size_t capacity;
	size_t counter;
	size_t token_count;
} String;

typedef struct {
	char const* buffer;
	size_t len;
} StringSlice;

void string_push(Arena* a, String* str, char c);
void string_append(Arena* a, String* str1, const String* str2);
void string_append_c_str(Arena* a, String* str, const char* c_str);
void string_append_slice(Arena* a, String* str, const StringSlice* slice);
void string_append_int(Arena* a, String* str, int num);
void string_append_double_prec(Arena* a, String* str, double num, size_t precision);
#define string_append_double(a, str, num) string_append_double_prec(a, str, num, 5);
void string_read_file(Arena* a, String* str, const char* file_path);

#define string_get_slice(str, beginning, length) (StringSlice){ .buffer = (str)->buffer + beginning, .len = length }

char string_compare(const String* str1, const String* str2);

void string_remove(String* str, char c);
char string_parse_by(String* str, StringSlice* slice, char c);
char string_get_char(String* str);

#endif // DYNAMIC_STRING_H
