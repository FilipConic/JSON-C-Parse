#include "../include/arena.h"
#include "../include/json_value.h"
#include <stdlib.h>

char bst_add(Arena* a, BinarySearchTree* bst, JsonValue* key, JsonValue* value) {
	if (!bst->nul) {
		json_move(&bst->key, key);
		json_move(&bst->value, value);

		bst->left =  (BinarySearchTree*)arena_alloc(a, sizeof(BinarySearchTree));
		bst->right = (BinarySearchTree*)arena_alloc(a, sizeof(BinarySearchTree));

		bst->nul = 1;
		return 0;
	} else {
		char ret = string_compare(&bst->key.val.string, &key->val.string);
		if (ret == 0)
			return 1;
		
		if (ret > 0) {
			if (bst_add(a, bst->left, key, value) == 0)
				return 0;
			else 
				return 1;
		} else {
			if (bst_add(a, bst->right, key, value) == 0)
				return 0;
			else
			 	return 1;
		}
	}
}
char bst_add_c(Arena* a, BinarySearchTree* bst, const char* key, JsonValue* value) {
	JsonValue json_key = { 0 };
	json_key.type = JsonString;
	string_append_c_str(a, &json_key.val.string, key);
	char ret = bst_add(a, bst, &json_key, value);

	return ret;
}
JsonValue* bst_get_by_key(BinarySearchTree* bst, const String* key) {
	char ret = string_compare(&bst->key.val.string, key);
	if (ret == 0) {
		return &bst->value;
	} else if (ret > 0) {
		if (bst->left)
			return bst_get_by_key(bst->left, key);
		else 
			return NULL;
	} else {
		if (bst->right)
			return bst_get_by_key(bst->right, key);
		else 
			return NULL;
	}
}
JsonValue* bst_get_by_key_c(Arena* a, BinarySearchTree* bst, const char* key) {
	String help = { 0 };

	string_append_c_str(a, &help, key);
	JsonValue* ret = bst_get_by_key(bst, &help);

	return ret;
}
