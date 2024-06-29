#pragma once

/*
   libbce v1.3

   https://github.com/wr7/bce

   This software is licensed under the MIT No Attribution License (SPDX MIT-0).

   Copyright © 2024 wr7

   Permission is hereby granted, free of charge, to any person obtaining a copy 
   of this software and associated documentation files (the "Software"), to 
   deal in the Software without restriction, including without limitation the 
   rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
   sell copies of the Software, and to permit persons to whom the Software is 
   furnished to do so.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
   IN THE SOFTWARE.
 */

 /*
  * libbce is a single-header library for generating C headers/sources with 
  * embedded constants/variables.
  *
  * To use libbce, write a single source file with the following contents:
    ```
    	#define  LIBBCE_IMPL
    	#include "libbce.h"
    ```
  *
  * To generate a simple 3-variable file with libbce, you can just do:
    ```
    	#define ARRAY_LENGTH(arr) (sizeof(arr)/sizeof((arr)[0]))

    	// variables to write
    	const char *hello_world = "hello world!";
    	const unsigned char nums[] = {8, 64, 11, 92, 129, 2, 2, 55};
    	const int number = 69;

    	bce_file *file = bce_create("output.c");

    	bce_printf(file, "const char *hello_world=");
    	bce_print_string(file, hello_world, strlen(hello_world));
    	bce_printf(file, ";\n");

    	bce_printf(file, "const char *nums=");
    	bce_print_string(file, (const char*) &nums[0], ARRAY_LENGTH(nums));
    	bce_printf(file, ";\n");

    	bce_printf(file, "const int number=%d;\n", number);

    	bce_close(file);
    ```
  *
  * This will output the following to `output_file.c`:
  * ```
  * const char *hello_world="hello world!";
  * const unsigned char nums[]="\b@\v\\\201\2\0027";
  * const int number = 69;
  * ```
  *
  * This example does not do any error handling, but most falliable libbce 
  * functions will return `false` and set `errno` upon error.
  *
  * Additionally, If all of the file-writing logic is put into a 
  * `bool`-returning function, the `bce_printfh` macro can be used which will 
  * automatically pass on any errors returned by `bce_printf`.
  */

#include <stdio.h>
#include <stddef.h>

typedef struct bce_file bce_file;

/**
 * Insert `data` as an array of unsigned char literals
 *
 * The variable type MUST be an `unsigned char[]` or this will not work 
 * properly!
 */
_Bool bce_print_byte_array(bce_file *file, const unsigned char *data, size_t len);

/**
 * Insert `data` as an array of signed char literals
 *
 * The variable type MUST be a `char[]` or this will not work properly!
 */
_Bool bce_print_signed_byte_array(bce_file *file, const char *data, size_t len);

/**
 * Insert `data` as an escaped string (eg "\0fbd\b\n\101"). The 
 * produced literal may be too long for some compilers.
 *
 * The variable type may be an array of or pointer to `char` or 
 * `unsigned char`.
 */
_Bool bce_print_string(bce_file *file, const char *data, size_t len);

/**
 * Insert `data` as an array of escaped strings. This is to get around 
 * string literal length limitations. 
 *
 * The variable type MUST be an `unsigned char *` or this will not work 
 * properly!
 */
_Bool bce_print_string_array(bce_file *file, const unsigned char *data, size_t len);

/**
 * Insert `data` as an array of escaped strings. This is to get around 
 * string literal length limitations. 
 *
 * The variable type MUST be a `char *` or this will not work properly!
 */
_Bool bce_print_signed_string_array(bce_file *file, const char *data, size_t len);

// Functions //

/**
 * Directly write data to a `bce_file`. This has the same syntax as fprintf.
 */
_Bool bce_printf(bce_file *file, const char *format, ...);

/**
 * Directly write data to a `bce_file`. This has the same syntax as fprintf. 
 * This expands to an if statement and will `return false` upon error.
 */
#define bce_printfh(...) if(!bce_printf(__VA_ARGS__)) {return false;}

/**
 * Creates a bce file with the name `filename`. This file must be closed with 
 * `bce_close`.
 *
 * Returns `NULL` and sets errno upon error.
 */
bce_file *bce_create(const char *filename);

/**
 * Creates a bce file from a `FILE *`. This must be closed with `bce_close`.
 */
bce_file *bce_create_from_file(FILE *file);

/**
 * Creates a bce file in-memory. A pointer to the file will be written to `buf`.
 */
bce_file *bce_create_in_memory(char **buf);

/**
 * Closes the file and frees its associated memory.
 *
 * Returns `false` and sets errno upon error.
 */
_Bool bce_close(bce_file *file);

#ifdef LIBBCE_IMPL

#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>

#include <string.h>
#include <stdbool.h>

#define ARR_LEN(arr) (sizeof(arr)/sizeof((arr)[0]))

typedef struct {
	char **data;
	size_t length;
	size_t capacity;
} allocated_string;

struct bce_file {
	bool is_file;
	union {
		FILE *file;
		allocated_string string;
	} u;
};

bool bce_printf(bce_file *file, const char *format, ...) {
	va_list va;

	if(file->is_file) {
		va_start(va, format);
		int res = vfprintf(file->u.file, format, va);
		va_end(va);

		return res >= 0;
	} 
	
	allocated_string *str = &file->u.string;

	va_start(va, format);
	const int len = vsnprintf(NULL, 0, format, va);
	va_end(va);

	if(len < 0) 
		return false;

	const size_t orig_capacity = str->capacity;
	while((size_t) len > str->capacity - str->length - 1) {
		str->capacity *= 2;
	}

	if(str->capacity != orig_capacity)
		*str->data = realloc(*str->data, str->capacity);

	va_start(va, format);
	vsprintf(*str->data + str->length, format, va);
	va_end(va);

	str->length += len;

	return true;
}

bce_file *bce_create(const char *const filename) {
	FILE *file = fopen(filename, "wb");

	if(!file)
		return NULL;

	return bce_create_from_file(file);
}

bce_file *bce_create_from_file(FILE *file) {
	bce_file *bfile = malloc(sizeof(*bfile));

	*bfile = (bce_file) {
		.is_file = true,
		.u.file = file,
	};

	return bfile;
}

bce_file *bce_create_in_memory(char **buf) {
	bce_file *bfile = malloc(sizeof(*bfile));

	*bfile = (bce_file) {
		.is_file = false,
		.u.string = (allocated_string) {
			.data = buf,
			.capacity = 64,
			.length = 0,
		},
	};

	*buf = malloc(bfile->u.string.capacity);
	(*buf)[0] = '\0';

	return bfile;
}

bool bce_close(bce_file *p_file) {
	bce_file file = *p_file;
	free(p_file);

	if(file.is_file) {
		if(fclose(file.u.file) < 0) {
			return false;
		}
	}

	return true;
}

bool bce_print_string(bce_file *file, const char *string, size_t len) {
	bce_printfh(file, "\"");

	for(size_t i = 0; i < len; i++) {
		const char c = string[i];

		// Characters that need to be escaped //
		if(c == '\\' || c == '"' || c == '?') {
			bce_printfh(file, "\\");
		}

		// Printable characters //
		if(c >= ' ' && c <= '~') {
			bce_printfh(file, "%c", c);
			continue;
		}

		// Non-printable characters //
		bce_printfh(file, "\\");

		// Non-printable characters w/ dedicated escape codes //
		const char other_escapes[] = {'a', 'b', 't', 'n', 'v', 'f', 'r'};

		if(c >= '\a' && c <= '\r') {
			bce_printfh(file, "%c", other_escapes[c - '\a']);
			continue;
		}

		// Display character as octal

		const unsigned char uc = (unsigned char) c;

		if(i + 1 < len) { // if the next char is a digit, we must pad octal to 3 digits
			const char next_char = string[i + 1];
			if(next_char >= '0' && next_char <= '9') {
				bce_printfh(file, "%03o", uc);
				continue;
			}
		}

		bce_printfh(file, "%o", uc);
	}

	bce_printfh(file, "\"");

	return true;
}

static bool _bce_print_byte_array(bce_file *file, const char *string, size_t len, bool is_signed) {
	bce_printfh(file, "{");

	for(size_t i = 0; i < len; i++) {
		if(i % 500 == 0 && i != 0)
			bce_printfh(file, "\n\t");

		if(is_signed) {
			bce_printfh(file, "%d,", string[i]);
		} else {
			bce_printfh(file, "%u,", (unsigned char) string[i]);
		}
	}

	bce_printfh(file, "}");
	return true;
}

bool bce_print_byte_array(bce_file *file, const uint8_t *string, size_t len) {
	return _bce_print_byte_array(file, (char *) string, len, false);
}

bool bce_print_signed_byte_array(bce_file *file, const char *string, size_t len) {
	return _bce_print_byte_array(file, string, len, true);
}

// Ceiling division
static size_t size_t_cdiv(size_t num, size_t den) {
    return (num + den - 1) / den;
}

static size_t size_t_min(size_t a, size_t b) {
	if(a < b) {
		return a;
	} else {
		return b;
	}
}

static bool _bce_print_string_array(bce_file *file, const uint8_t *string, size_t len, bool is_signed) {
	const char *type = is_signed ? "char" : "unsigned char";
	bce_printfh(file, "(%s *) (char[][%zu]) {", type, size_t_min(len, 500));

	for(size_t i = 0; i < size_t_cdiv(len, 500); i++) {
		size_t len_to_write = len - i * 500;
		if(len_to_write > 500)
			len_to_write = 500;

		bce_print_string(file, (char *) (string + 500 * i), len_to_write);
		bce_printfh(file, ",\n\t");
	}

	bce_printfh(file, "}");
	return true;
}

bool bce_print_string_array(bce_file *file, const uint8_t *string, size_t len) {
	return _bce_print_string_array(file, string, len, false);
}

bool bce_print_signed_string_array(bce_file *file, const char *string, size_t len) {
	return _bce_print_string_array(file, (uint8_t *) string, len, true);
}

#endif
