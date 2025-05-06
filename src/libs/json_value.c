#include "../include/json_value.h"

#include <ctype.h>
#include <stdio.h>
#include <math.h>

#define is_whitespace(c) ((c) == '\n' || (c) == ' ' || (c) == '\t' || (c) == '\r')

void json_parse_number(String* str, JsonValue* json) {
	enum NumberParseFST {
		Error,
		Start,
		Zero,
		Digit1_9,
		Digit,
		Point,
		PointDigit,
		Exp,
		MinusPlus,
		ExpDigit,
		End,
	};
	const char fst[End][15] = {
	    //0  1  2  3  4  5  6  7  8  9  -  + e/E .  End
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0  }, // Error
		{ 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0,  0  }, // Start
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 5, End }, // Zero
		{ 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 7, 5, End }, // Digit1_9
		{ 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 7, 5, End }, // Digit
		{ 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 0, 0,  0  }, // Point
		{ 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 7, 0, End }, // PointDigit
		{ 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 8, 8, 0, 0,  0  }, // Exp
		{ 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 0, 0, 0, 0,  0  }, // MinusPlus
		{ 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 0, 0, 0, 0, End }, // ExpDigit
	};
	--str->token_count;
	char c = string_get_char(str);
	char is_neg;
	if (c == '-') {
		is_neg = 1;
	} else {
		is_neg = 0;
		--str->token_count;
	}

	int read_num = 0;
	double num = 0;
	double frac = 1;
	double exp = 0;
	char is_exp_neg = 0;
	enum NumberParseFST curr = Start;
	while (curr != End) {
		c = string_get_char(str);
		switch (c) {
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9': read_num = c - '0'; break;
			case '-': read_num = 10;      break;
			case '+': read_num = 11;      break;
			case 'e':
			case 'E': read_num = 12;      break;
			case '.': read_num = 13;      break;
			default:  read_num = 14;      break;
		}
		switch((curr = fst[curr][read_num])) {
			case Start:
			case Point:
			case Exp:
			case End:
			case Zero: break;
			case MinusPlus:
				if (read_num == 10) is_exp_neg = 1;
				else is_exp_neg = 0;
				break;
			case PointDigit:
				frac *= 10.0;
				num = num * 10.0 + (double)read_num; 
				break;
			case Digit1_9:
			case Digit:
				num = num * 10.0 + (double)read_num; 
				break;
			case ExpDigit:
				exp = exp * 10.0 + (double)read_num;
				break;
			case Error:
				fprintf(stderr, "ERROR: There was an error while parsing a number at point %zu:\n", str->token_count);
				for (size_t i = str->token_count; i < str->counter && i < str->token_count + 25; ++i) {
					putc(str->buffer[i], stderr);
				}
				putc('\n', stderr);
				fprintf(stderr, "Read char is %d and it turns into the state %d\n", (int)c, read_num);
				return;
		}
	}
	json->type = JsonNumber;
	json->val.number = (is_neg ? -1 : 1) * num / frac * pow(10, exp * (is_exp_neg ? -1 : 1));
	--str->token_count;
}
void json_parse_string(Arena* a, String* str, JsonValue* json) {
	enum StringParseFST {
		Error,
		Start,
		ReverseSolidus,
		U,
		U1,
		U2,
		U3,
		End,
	};
	const char fst[End][10] = {
	//   XYZ \  bfnrt  u  num  "
		{ 0, 0,   0,   0,  0,  0  }, // Error
		{ 1, 2,   1,   1,  1, End }, // Start
		{ 0, 1,   1,   3,  0,  1  }, // ReverseSolidus
		{ 0, 0,   0,   0,  4,  0  }, // U
		{ 0, 0,   0,   0,  5,  0  }, // U1
		{ 0, 0,   0,   0,  6,  0  }, // U2
		{ 0, 0,   0,   0,  1,  0  }, // U3
	};
	json->type = JsonString;
	json->val.string.counter = 0;

	int type = 0;
	enum StringParseFST curr = Start;
	char c = 0;
	char next_special = 0;

	while (curr != End) {

		switch ((c = string_get_char(str))) {
			case '\\': type = 1; break;
			case 'b':
			case 'f':
			case 'n':
			case 'r':
			case 't': type = 2; break;
			case 'u': type = 3; break;
			case '\"': type = 5; break;
			default:
				if (isdigit(c) || (c >= 'a' && c <= 'f')) {
					type = 4;
				} else if (c != '\0') {
					type = 0;
				} else {
					json->type = JsonNull;
					
					fprintf(stderr, "ERROR: There was an error while parsing a string at point %zu:\n", str->token_count);
					for (size_t i = str->token_count; i < str->counter && i < str->token_count + 25; ++i) {
						putc(str->buffer[i], stderr);
					}
					putc('\n', stderr);
					return;
				}
				break;
		}
		switch ((curr = fst[curr][type])) {
			case Error:
				fprintf(stderr, "ERROR: There was an error while parsing a string at point %zu:\n", str->token_count);
				for (size_t i = str->token_count; i < str->counter && i < str->token_count + 25; ++i) {
					putc(str->buffer[i], stderr);
				}
				putc('\n', stderr);
				return;
			case End:
				break;
			case Start:
				if (!next_special) {
						string_push(a, &json->val.string, c);
				} else {
					if (c == 'b') string_push(a, &json->val.string, '\b'); 
					else if (c == 'n') string_push(a, &json->val.string, '\n'); 
					else if (c == 'f') string_push(a, &json->val.string, '\f'); 
					else if (c == 'r') string_push(a, &json->val.string, '\r'); 
					else if (c == 't') string_push(a, &json->val.string, '\t'); 
					else if (c == '\\') string_push(a, &json->val.string, '\\'); 
					else if (c == '\"') string_push(a, &json->val.string, '\"'); 
					next_special = 0;
				}
				break;
			case ReverseSolidus:
				next_special = 1;
				break;
			case U:
				next_special = 0;
				string_push(a, &json->val.string, '\\');
				string_push(a, &json->val.string, 'u');
				break;
			case U1:
			case U2:
			case U3:
				string_push(a, &json->val.string, c);
				break;
		}
	}
	// printf("End: %d, pos: %zu, string: %s\n", (int)c, str->token_count, json->val.string.buffer);
}
void json_array_expand(Arena* a, JsonValue* json) {
	if (json->type != JsonArray) {
		fprintf(stderr, "ERROR: Trying to array expand a non array value!\n");
		return;
	}
	if (json->val.array.capacity) {
		json->val.array.buffer = arena_realloc(a, json->val.array.buffer, json->val.array.counter, json->val.array.capacity <<= 1);
	} else {
		json->val.array.buffer = arena_alloc(a, 8 * sizeof(JsonValue));
		json->val.array.capacity = 8;
		json->val.array.counter = 0;
	}
}
void json_parse_array(Arena* a, String* str, JsonValue* json) {
	enum ArrayParseFST {
		Error,
		Start,
		Value,
		Whitespace,
		Comma,
		End,
	};
	const char fst[End][4] = {
		//\t  , abc ]
		{  0, 0, 0, 0 }, // Error
		{  1, 0, 2,End}, // Start
		{  3, 4, 0,End}, // Value
		{  3, 4, 0,End}, // Whitespace
		{  1, 0, 2, 0 }, // Comma
	};
	json->type = JsonArray;
	enum ArrayParseFST curr = Start;
	JsonValue val = { 0 };
	int type = 0;
	char c = 0;

	while (curr != End) {
		switch (c = string_get_char(str)) {
			case '\t':
			case ' ':
			case '\n':
			case '\r': type = 0; break;
			case ',': type = 1; break;
			case ']': type = 3; break;
			default: type = 2;
		}
		switch (curr = fst[curr][type]) {
			case Error:
				fprintf(stderr, "ERROR: There has been a problem while parsing an array at position: %zu\n", str->token_count);
				for (size_t i = str->token_count; i < str->counter && i < str->token_count + 25; ++i) {
					putc(str->buffer[i], stderr);
				}
				putc('\n', stderr);
				json->type = JsonNull;
				return;
			case Start:
			case Whitespace:
			case End:
			case Comma: break;
			case Value:
				--str->token_count;
				json_parse(a, str, &val);
				if (json->val.array.counter == json->val.array.capacity)
					json_array_expand(a, json);
				json_move(&json->val.array.buffer[json->val.array.counter++], &val);
				break;

		}
	}
	json->val.array.capacity = json->val.array.counter;
}
void json_parse_object(Arena* a, String* str, JsonValue* json) {
	enum ObjectParseFST {
		Error,
		Start,
		EnumString,
		WhiteSpace1,
		Colon,
		Value,
		WhiteSpace2,
		Comma,
		End,
	};
	const char fst[End][6] = {
		//\t  "  :  ,  }  abc
		{  0, 0, 0, 0,  0, 0 }, // Error
		{  1, 2, 0, 0,End, 0 }, // Start
		{  3, 0, 4, 0,  0, 0 }, // EnumString 
		{  3, 0, 4, 0,  0, 0 }, // WhiteSpace1
		{  5, 5, 0, 0,  0, 5 }, // Colon
		{  6, 0, 0, 7,End, 0 }, // Value
		{  6, 0, 0, 7,End, 0 }, // WhiteSpace2
		{  2, 2, 0, 0,  0, 0 }  // Comma
	};
	char c = 0;
	enum ObjectParseFST curr = Start;
	int type = 0;
	JsonValue key = { 0 }, val = { 0 };
	json->type = JsonObject;
	json->val.object = (BinarySearchTree*)arena_alloc(a, sizeof(BinarySearchTree));
	while (curr != End) {
		switch (c = string_get_char(str)) {
			case '\t':
			case '\r':
			case '\n':
			case ' ': type = 0; break;
			case '\"': type = 1; break;
			case ':': type = 2; break;
			case ',': type = 3; break;
			case '}': type = 4; break;
			default: type = 5; break;
		}
		enum ObjectParseFST new_curr = fst[curr][type];
		// printf("State change: %d -> %d, type: %d, char: %d, pos: %zu\n", curr, new_curr, type, (int)c, str->token_count);
		curr = new_curr;
		switch (curr) {
			case EnumString:
				if (str->token_count) --str->token_count;
				json_parse(a, str, &key);
				if (key.type != JsonString) {
					fprintf(stderr, "ERROR: Didn\'t find a string as the key, but the %d, in the object at pos: %zu!\n", key.type, str->token_count);
					for (size_t i = str->token_count; i < str->counter && i < str->token_count + 25; ++i) {
						putc(str->buffer[i], stderr);
					}
					putc('\n', stderr);
					json->type = JsonNull;
					return;
				}
				break;	
			case Value:
				--str->token_count;
				json_parse(a, str, &val);
				bst_add(a, json->val.object, &key, &val);
				break;
			case End:
			case Colon:
			case Comma:
			case WhiteSpace1:
			case WhiteSpace2:
			case Start: break;
			case Error:
				fprintf(stderr, "ERROR: There was an error while parsing an object at point %zu:\n", str->token_count);
				for (size_t i = str->token_count; i < str->counter && i < str->token_count + 25; ++i) {
					putc(str->buffer[i], stderr);
				}
				putc('\n', stderr);
				json->type = JsonNull;
				return;
		}
	}
}

void json_parse(Arena* a, String* str, JsonValue* json) {
	enum ParseFST {
		Error,
		Start,
		N,
		Nu,
		Nul,
		Null,
		T,
		Tr,
		Tru,
		True,
		F,
		Fa,
		Fal,
		Fals,
		False,
		Quotes,
		Num,
		LeftSqBr,
		LeftBr,
		End,
	};
	const char fst[End][15] = {
		//* \t  n  u  l  t  r  e  f  a  s  "  [  {  -num
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // Error
		{ 0, 1, 2, 0, 0, 6, 0, 0,10, 0, 0,15,17,18,16 }, // Start
		{ 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // N
		{ 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // Nu
		{ 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // Nul
		{ 0 }, // Null
		{ 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0 }, // T
		{ 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // Tr
		{ 0, 0, 0, 0, 0, 0, 0, 9, 0, 0, 0, 0, 0, 0, 0 }, // Tru
		{ 0 }, // True
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0,11, 0, 0, 0, 0, 0 }, // F
		{ 0, 0, 0, 0,12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // Fa
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,13, 0, 0, 0, 0 }, // Fal
		{ 0, 0, 0, 0, 0, 0, 0,14, 0, 0, 0, 0, 0, 0, 0 }, // Fals
		{ 0 }, // False
		{ 0 }, // Quotes
		{ 0 }, // Num
		{ 0 }, // LeftSqBr
		{ 0 }  // LeftBr
	};

	enum ParseFST curr = Start;
	int type;
	char c = 0;
	while (curr != End) {
		switch (c = string_get_char(str)) {
			case '\t':
			case ' ':
			case '\r':
			case '\n': type = 1; break;
			case 'n': type = 2; break;
			case 'u': type = 3; break;
			case 'l': type = 4; break;
			case 't': type = 5; break;
			case 'r': type = 6; break;
			case 'e': type = 7; break;
			case 'f': type = 8; break;
			case 'a': type = 9; break;
			case 's': type = 10; break;
			case '\"': type = 11; break;
			case '[': type = 12; break;
			case '{': type = 13; break;
			case '-': case '0':
			case '1': case '2': case '3':
			case '4': case '5': case '6':
			case '7': case '8': case '9': type = 14; break;
			default: type = 0; break;
		}
		switch (curr = fst[curr][type]) {
			case N: case Nu: case Nul:
			case T: case Tr: case Tru:
			case F: case Fa: case Fal: case Fals:
			case Start: case End: break;
			case Null:
				json->type = JsonNull;
				json->val.empty = 0;
				curr = End;
				break;
			case True:
				json->type = JsonTrue;
				json->val.empty = 0;
				curr = End;
				break;
			case False:
				json->type = JsonFalse;
				json->val.empty = 0;
				curr = End;
				break;
			case Num:
				json_parse_number(str, json);
				curr = End;
				break;
			case Quotes:
				json_parse_string(a, str, json);
				curr = End;
				break;
			case LeftSqBr:
				json_parse_array(a, str, json);
				curr = End;
				break;
			case LeftBr:
				json_parse_object(a, str, json);
				curr = End;
				break;
			case Error:
				fprintf(stderr, "ERROR: There was an error while trying to parse JSON at point %zu:\n", str->token_count);
				for (size_t i = str->token_count; i < str->counter && i < str->token_count + 25; ++i) {
					putc(str->buffer[i], stderr);
				}
				putc('\n', stderr);
				json->type = JsonNull;
				json->val.empty = 0;
				return;
		}
	}
	return;
}
void json_to_string(Arena* a, JsonValue* json, String* str);

void json_move(JsonValue* dst, JsonValue* src) {
	dst->type = src->type;
	src->type = JsonNull;
	switch (dst->type) {
		case JsonTrue:
		case JsonNull:
		case JsonFalse:
			dst->val.empty = 0;
			break;
		case JsonNumber:
			dst->val.number = src->val.number;
			src->val.number = 0;
			break;
		case JsonString:
			dst->val.string = src->val.string;
			src->val.string = (String){ 0 };
			break;
		case JsonArray:
			dst->val.array = src->val.array;
			src->val.array.buffer = NULL;
			src->val.array.capacity = 0;
			src->val.array.counter = 0;
			break;
		case JsonObject:
			dst->val.object = src->val.object;
			src->val.object = NULL;
			break;
	}
}
