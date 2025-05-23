#include "../include/dynamic_string.h"

#include <assert.h>
#include <stdarg.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRING_INIT_LEN 256

void expand_string(Arena* a, String* str) {
	if (str->buffer) {
		str->buffer = (char*)arena_realloc(a, str->buffer, str->capacity, str->capacity * 1.5);
		str->capacity = (size_t)(str->capacity * 1.5);
	} else {
		string_reserve(a, str, STRING_INIT_LEN);
		str->counter = 0;
	}
}

void string_reserve(Arena* a, String *str, size_t num) {
	if (str->buffer) {
		str->buffer = (char*)arena_realloc(a, str->buffer, str->capacity, num);
		str->buffer[num - 1] = '\0';
	} else {
		str->buffer = (char*)arena_alloc(a, num);
	}
	str->capacity = num;
}
void string_push(Arena* a, String* str, char c) {
	if (str->counter + 1 >= str->capacity)
		expand_string(a, str);

	str->buffer[str->counter++] = c;
	str->buffer[str->counter] = '\0';
}
void string_append(Arena* a, String* str1, const String* str2) {
	while (str1->counter + str2->counter + 1 >= str1->capacity)
		expand_string(a, str1);
	memcpy(str1->buffer + str1->counter, str2->buffer, str2->counter);
	str1->counter += str2->counter;
	str1->buffer[str1->counter] = '\0';
}
void string_append_c_str(Arena* a, String* str, const char* c_str) {
	size_t size = strlen(c_str) + 1;
	while (size + str->counter >= str->capacity)
		expand_string(a, str);
	memcpy(str->buffer + str->counter, c_str, size * sizeof(char));
	str->counter += size - 1;
}
void string_append_slice(Arena* a, String* str, const StringSlice* slice) {
	while (str->counter + slice->len + 1 >= str->capacity)
		expand_string(a, str);
	memcpy(str->buffer + str->counter, slice->buffer, slice->len);
	str->counter += slice->len;
	str->buffer[str->counter] = '\0';
}
void string_append_format(Arena* a, String* str, const char* format, ...) {
	int n = 0;
	for (size_t i = 0; format[i]; ++i) {
		if (format[i] == '%') {
			if (format[i + 1] != '%') {
				++n;
			}
		}
	}

	va_list args;
	va_start(args, format);
	
	char percent_found = 0;
	char is_long = 0;
	for (size_t i = 0; format[i]; ++i) {
		if (percent_found) {
#define sub_n() do { \
				if (--n < 0) { \
					fprintf(stderr, "ERROR: Too many parameters given to variadic function string_append_format!\n"); \
					assert(0); \
				} \
			} while (0)
			switch (format[i]) {
				case '%':
					string_push(a, str, '%');
					percent_found = 0;
					sub_n();
					break;
				case 'l':
				case 'L':
					if (is_long == 0 || is_long == 1) {
						++is_long;
					} else {
						fprintf(stderr, "ERROR: You can\'t have more than two Ls given as a format for a number!\n");
						assert(0);
					}
					break;
				case 'd':
				case 'i':
				case 'u': {
					long long int num = is_long ? va_arg(args, long long int) : va_arg(args, int);
					string_append_int(a, str, num);
					is_long = 0;
					percent_found = 0;
					sub_n();
					break;
				}
				case 'f':
				case 'F':
				case 'e':
				case 'E': {
					double num = va_arg(args, double);
					string_append_double(a, str, num);
					is_long = 0;
					percent_found = 0;
					sub_n();
					break;
				}
				case 'c': {
					if (is_long) {
						fprintf(stderr, "ERROR: You can\'t turn char into a long!\n");
						assert(0);
					}
					char c = (char)va_arg(args, int);
					string_push(a, str, c);
					percent_found = 0;
					sub_n();
					break;
				}
				case 's': {
					if (is_long) {
						fprintf(stderr, "ERROR: You can\'t turn string into a long!\n");
						assert(0);
					}
					char* s = va_arg(args, char*);
					string_append_c_str(a, str, s);
					percent_found = 0;
					sub_n();
					break;
				}
				case 'S': {
					if (is_long) {
						fprintf(stderr, "ERROR: You can\'t turn string into a long!\n");
						assert(0);
					}
					String s = va_arg(args, String);
					string_append(a, str, &s);
					percent_found = 0;
					sub_n();
					break;
				}
				default:
					fprintf(stderr, "ERROR: Character [%c] not supported!\n", format[i]);
					assert(0);
			}
		} else {
			if (format[i] == '%') {
				percent_found = 1;
			} else {
				string_push(a, str, format[i]);
			}
		}
	}
	va_end(args);

	if (n != 0) {
		fprintf(stderr, "ERROR: Not all %% used in variadic function string_append_format!\n");
	}
}
void string_append_int(Arena* a, String* str, long long int num) {
	size_t i = str->counter;
	if (num == 0) {
		string_push(a, str, '0');
		return;
	}
	if (num < 0) {
		++i;
		string_push(a, str, '-');
		num = -num;
	}
	while (num != 0) {
		char n = num % 10;
		num = (num - n) / 10;
		string_push(a, str, n + '0');
	}
	for (size_t j = str->counter - 1; i < j; ++i, --j) {
		str->buffer[i] ^= str->buffer[j];
		str->buffer[j] ^= str->buffer[i];
		str->buffer[i] ^= str->buffer[j];
	}
}
void string_append_double_prec(Arena* a, String* str, double num, size_t precision) {
	if (num < 0) {
		string_push(a, str, '-');
		num = -num;
	}
	if (num < 1e-5 || num > 1e9) {
		int exp = (int)log10(num) - (num > 1e9 ? 0 : 1);
		double work_num = num * pow(10, -exp);
		
		int whole_part = (int)floor(work_num);
		work_num -= (double)whole_part;

		string_append_int(a, str, whole_part);
		string_push(a, str, '.');
		string_append_int(a, str, (int)(work_num * pow(10, precision)));

		string_push(a, str, 'e');
		string_append_int(a, str, exp);
	} else {
		int whole_part = (int)floor(num);
		num -= (double)whole_part;

		string_append_int(a, str, whole_part);
		string_push(a, str, '.');
		string_append_int(a, str, (int)(num * pow(10, precision)));
	}
}
void string_read_file(Arena* a, String* str, const char* file_path) {
	FILE* file = fopen(file_path, "r");
	if (!file) {
		fprintf(stderr, "ERROR: File doesn\'t exists or can\'t be opened!\n");
		return;
	}
	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);
	fseek(file, 0, SEEK_SET);

	while (str->counter + size + 1 >= str->capacity)
		expand_string(a, str);

	fread(str->buffer + str->counter, sizeof(char), size, file);
	str->counter += size;
	str->buffer[str->counter] = '\0';

	fclose(file);
}

char string_compare(const String* str1, const String* str2) {
	if (str1->counter < str2->counter) {
		int ret = strncmp(str1->buffer, str2->buffer, str1->counter);
		if (ret <= 0) return -1;
		else return 1;
	} else if (str1->counter > str2->counter) {
		int ret = strncmp(str1->buffer, str2->buffer, str2->counter);
		if (ret >= 0) return 1;
		else return -1;
	} else {
		int ret = strncmp(str1->buffer, str2->buffer, str1->counter);
		if (ret < 0) return -1;
		else if (ret > 0) return 1;
		else return 0;
	}
	return 0;
}

void string_remove(String* str, char c) {
	size_t removed_count = 0;
	for (size_t i = 0; i < str->counter; ++i) {
		if (c == str->buffer[i]) {
			++removed_count;
		} else {
			str->buffer[i - removed_count] = str->buffer[i];
		}
	}
	str->counter -= removed_count;
	str->buffer[str->counter] = '\0';
}
char string_parse_by(String* str, StringSlice* slice, char c) {
	size_t counter = 0;
	for (size_t i = str->token_count; i < str->counter; ++i, ++counter) {
		if (str->buffer[i] == c) 
			break;
	}
	slice->buffer = str->buffer + str->token_count;
	slice->len = counter;
	str->token_count += counter + 1;
	if (str->token_count >= str->counter) {
		str->token_count = 0;
		return 1;
	}
	return 0;
}
char string_get_char(String* str) {
	if (str->token_count == str->counter) { return '\0'; }

	return str->buffer[str->token_count++];
}
