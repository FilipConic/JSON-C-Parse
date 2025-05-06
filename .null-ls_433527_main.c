#include <stdio.h>

#include "./src/include/json_value.h"

void print_string_slice(StringSlice* slice) {
	for (size_t i = 0; i < slice->len; ++i) {
		putc(slice->buffer[i], stdout);
	}
}
void print_json_val(JsonValue* json) {
	switch(json->type) {
		case JsonNull:
			printf("found null!\n");
			break;
		case JsonTrue:
			printf("found true!\n");
			break;
		case JsonFalse:
			printf("found false!\n");
			break;
		case JsonNumber:
			printf("number: %lf\n", json->val.number);
			break;
		case JsonString:
			printf("string: %s\n", json->val.string.buffer);
			break;
		case JsonArray:
			printf("array = size: %zu\n=========================\n", json->val.array.counter);
			for (size_t i = 0; i < json->val.array.counter; ++i) {
				print_json_val(&json->val.array.buffer[i]);
			}
			printf("=========================\n");
			break;
		case JsonObject:
			printf(">=---------------------=<\n");
			printf("key:\n");
			print_json_val(&json->val.object->key);
			printf(">=---------------------=<\n");
			printf("value:\n");
			print_json_val(&json->val.object->value);
			printf(">=---------------------=<\n");
			if (json->val.object->left->nul) {
				printf("left: \n");
				JsonValue left = {
					.type = JsonObject,
					.val.object = json->val.object->left,
				};
				print_json_val(&left);
			}
			if (json->val.object->right->nul) {
				printf("right: \n");
				JsonValue right = {
					.type = JsonObject,
					.val.object = json->val.object->right,
				};
				print_json_val(&right);
			}
			printf(">=---------------------=<\n");
			break;
		default:
			printf("didn\'t find anything\n");
	}
}

int main(int argc, char** argv) {
	if (argc < 2) {
		fprintf(stderr, "ERROR: Not enough arguments given!\n");
		return 1;
	}
	Arena a = { 0 };
	arena_init(&a, 160 * 1024);
	String str = { 0 };
	string_read_file(&a, &str, argv[1]);

	// printf("%s\n", str.buffer);
	JsonValue json = { 0 };
	str.token_count = 0;
	json_parse(&a, &str, &json);
	printf("parse done! space taken: %zu\n", a.taken);
	
	// print_json_val(&json);
	if (json.type == JsonObject) {
		JsonValue* found = bst_get_by_key_c(&a, json.val.object, "subscriptions");
		if (found)
			print_json_val(found);
	}

	arena_free(&a);
	return 0;
}
