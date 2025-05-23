#ifndef JSON_VALUE_H
#define JSON_VALUE_H

#include <stdlib.h>
#include "dynamic_string.h"

enum JsonType {
	JsonNull,
	JsonTrue,
	JsonFalse,
	JsonNumber,
	JsonString,
	JsonArray,
	JsonObject,
};

struct BinarySearchTree;

typedef struct JsonValue {
	enum JsonType type;
	union {
		char empty;
		double number;
		String string;
		struct {
			struct JsonValue* buffer;
			size_t capacity;
			size_t counter;
		} array;
		struct BinarySearchTree* object;
	} val;
} JsonValue;

typedef struct BinarySearchTree {
	struct BinarySearchTree *left, *right;
	JsonValue key;
	JsonValue value;
	char nul;
} BinarySearchTree;

char bst_add(Arena* a, BinarySearchTree* bst, JsonValue* key, JsonValue* value);
char bst_add_c(Arena* a, BinarySearchTree* bst, const char* key, JsonValue* value);
JsonValue* bst_get_by_key(BinarySearchTree* bst, const String* key);
JsonValue* bst_get_by_key_c(Arena* a, BinarySearchTree* bst, const char* key);

void json_parse(Arena* a, String* str, JsonValue* json);
void json_to_string_depth(Arena* a, JsonValue* json, String* str, int depth);
#define json_to_string(a, json, str) json_to_string_depth(a, json, str, 0);
#define json_string_append_true(a, str) do {\
	JsonValue json = { 0 }; \
	json.type = JsonTrue; \
	json_to_string_depth(a, &json, str, 0); \
} while (0)
#define json_string_append_false(a, str) do {\
	JsonValue json = { 0 }; \
	json.type = JsonFalse; \
	json_to_string_depth(a, &json, str, 0); \
} while (0)
void* json_get_value(JsonValue* json);
#define json_object_get(a, json, json_ret, str) do { \
	if ((json)->type != JsonObject) { \
		fprintf(stderr, "ERROR (LINE %d): Expected a JSON object!\n", __LINE__); \
	} \
	json_ret = bst_get_by_key_c(a, (json)->val.object, str); \
	if (!json_ret) { \
		fprintf(stderr, "ERROR (LINE %d): There is no value with the key [%s]!\n", __LINE__, str); \
	} \
} while (0)

void json_move(JsonValue* dst, JsonValue* src);

#endif // JSON_VALUE_H
