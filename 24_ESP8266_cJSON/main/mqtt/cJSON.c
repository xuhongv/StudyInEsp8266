/*
 * Fork form https://github.com/DaveGamble/cJSON
 * 锟斤拷锟斤拷学习cJSON
 * cJSON锟芥本锟斤拷 v1.6.0
 */

/*
 Copyright (c) 2009-2017 Dave Gamble and cJSON contributors

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */

/* cJSON */
/* JSON parser in C. */

/* disable warnings about old C89 functions in MSVC */
#if !defined(_CRT_SECURE_NO_DEPRECATE) && defined(_MSC_VER)
#define _CRT_SECURE_NO_DEPRECATE
#endif

#ifdef __GNUC__
#pragma GCC visibility push(default)
#endif
#if defined(_MSC_VER)
#pragma warning (push)
/* disable warning about single line comments in system headers */
#pragma warning (disable : 4001)
#endif

/************************************************************************************/
//
#include "cJSON.h"

#include <string.h>
#include <math.h>
#include <limits.h>

#ifndef CJSON_FOR_ESP8266
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#endif

#ifdef ENABLE_LOCALES
#include <locale.h>
#endif

#if defined(_MSC_VER)
#pragma warning (pop)
#endif
#ifdef __GNUC__
#pragma GCC visibility pop
#endif

/* define our own boolean type */
#ifndef true
#define true ((cJSON_bool)1)
#endif
#ifndef false
#define false ((cJSON_bool)0)
#endif

/************************************************************************************/
// 有关错误信息的代码
/* 错误信息结构体 */
typedef struct {
	const unsigned char *json;
	size_t position;
} error;
static error global_error = { NULL, 0 };

/*
 * function: cJSON_GetErrorPtr
 * description: 获取错误信息
 */
CJSON_PUBLIC(const char *) cJSON_GetErrorPtr(void)
{
	return (const char*) (global_error.json + global_error.position);
}

/************************************************************************************/
// 有关版本的代码
/* This is a safeguard to prevent copy-pasters from using incompatible C and header files */
#if (CJSON_VERSION_MAJOR != 1) || (CJSON_VERSION_MINOR != 6) || (CJSON_VERSION_PATCH != 0)
#error cJSON.h and cJSON.c have different versions. Make sure that both have the same.
#endif

/*
 * function: cJSON_Version
 * description: get version of cJSON
 */
CJSON_PUBLIC(const char*) cJSON_Version(void)
{
	static char version[15];
	os_sprintf(version, "%i.%i.%i", CJSON_VERSION_MAJOR, CJSON_VERSION_MINOR, CJSON_VERSION_PATCH);

	return version;
}

/************************************************************************************/

/*
 * function: case_insensitive_strcmp
 * description: Case insensitive string comparison, doesn't consider two NULL pointers equal though
 *              不区分大小写比较
 */
ICACHE_FLASH_ATTR static int case_insensitive_strcmp(
		const unsigned char *string1, const unsigned char *string2) {
	if ((string1 == NULL) || (string2 == NULL)) {
		return 1;
	}

	if (string1 == string2) {
		return 0;
	}

	for (; tolower(*string1) == tolower(*string2);
			(void) string1++, string2++) {
		if (*string1 == '\0') {
			return 0;
		}
	}

	return tolower(*string1) - tolower(*string2);
}

/************************************************************************************/
// 有关内存分配和释放的代码
// 内存分配和释放的钩子
typedef struct internal_hooks {
	void *(*allocate)(size_t size);
	void (*deallocate)(void *pointer);
	void *(*reallocate)(void *pointer, size_t size);
} internal_hooks;

// 定义cJSON中内存分配采用的方式
// 移植的时候可能需要修改

#if defined(_MSC_VER)
	/* work around MSVC error C2322: '...' address of dillimport '...' is not static */
	static void *internal_malloc(size_t size)
	{
		return malloc(size);
	}
	static void internal_free(void *pointer)
	{
		free(pointer);
	}
	static void *internal_realloc(void *pointer, size_t size)
	{
		return realloc(pointer, size);
	}
	#elif defined(CJSON_FOR_ESP8266)
	ICACHE_FLASH_ATTR static void *internal_malloc(size_t size) {
		return (void *) os_malloc(size);
	}
	ICACHE_FLASH_ATTR static void internal_free(void *pointer) {
		os_free(pointer);
	}
	ICACHE_FLASH_ATTR static void *internal_realloc(void *pointer, size_t size) {
		return (void *) os_realloc(pointer, size);
	}
#else
	#define internal_malloc malloc
	#define internal_free free
	#define internal_realloc realloc
#endif

static internal_hooks global_hooks = { internal_malloc, internal_free,
		internal_realloc };

/************************************************************************************/

/*
 * function: cJSON_strdup
 * description: 字符串拷贝函数，在构建object和array时会用到
 */
ICACHE_FLASH_ATTR static unsigned char* cJSON_strdup(
		const unsigned char* string, const internal_hooks * const hooks) {
	size_t length = 0;
	unsigned char *copy = NULL;

	// 检查参数
	if (string == NULL) {
		return NULL;
	}

	length = os_strlen((const char*) string) + sizeof("");
	copy = (unsigned char*) hooks->allocate(length);		// 申请内存
	if (copy == NULL) {
		return NULL;
	}

	os_memcpy(copy, string, length);

	return copy;
}

/*
 * function: cJSON_InitHooks
 * description: 初始化钩子函数
 */
CJSON_PUBLIC(void) cJSON_InitHooks(cJSON_Hooks* hooks) {
#ifndef CJSON_FOR_ESP8266
	if (hooks == NULL)
	{
		/* Reset hooks */
		global_hooks.allocate = malloc;
		global_hooks.deallocate = free;
		global_hooks.reallocate = realloc;
		return;
	}

	global_hooks.allocate = malloc;
	if (hooks->malloc_fn != NULL)
	{
		global_hooks.allocate = hooks->malloc_fn;
	}

	global_hooks.deallocate = free;
	if (hooks->free_fn != NULL)
	{
		global_hooks.deallocate = hooks->free_fn;
	}

	/* use realloc only if both free and malloc are used */
	global_hooks.reallocate = NULL;
	if ((global_hooks.allocate == malloc) && (global_hooks.deallocate == free))
	{
		global_hooks.reallocate = realloc;
	}
#else
	if (hooks->malloc_fn != NULL) {
		global_hooks.allocate = hooks->malloc_fn;
	}
	if (hooks->free_fn != NULL) {
		global_hooks.deallocate = hooks->free_fn;
	}
#endif
}

/************************************************************************************/

/*
 * function: cJSON_New_Item
 * description: 构造一个item，item可以理解为一个节点
 */
/* Internal constructor. */
ICACHE_FLASH_ATTR static cJSON *cJSON_New_Item(
		const internal_hooks * const hooks) {
	cJSON* node = (cJSON*) hooks->allocate(sizeof(cJSON));
	if (node) {
		os_memset(node, '\0', sizeof(cJSON));	// 锟斤拷始锟斤拷为0
	}

	return node;
}

/*
 * function: cJSON_Delete
 * description: 删除item，采用递归的方式释放item->child，循环方式释放item->next
 *              当root节点不使用后必须调用这个函数
 */
/* Delete a cJSON structure. */
CJSON_PUBLIC(void) cJSON_Delete(cJSON *item) {
	cJSON *next = NULL;
	while (item != NULL) {
		next = item->next;
		if (!(item->type & cJSON_IsReference) && (item->child != NULL)) {
			cJSON_Delete(item->child);  // 开始递归
		}
		if (!(item->type & cJSON_IsReference) && (item->valuestring != NULL)) {
			global_hooks.deallocate(item->valuestring);
		}
		if (!(item->type & cJSON_StringIsConst) && (item->string != NULL)) {
			global_hooks.deallocate(item->string);
		}
		global_hooks.deallocate(item);
		item = next;	// 获取下一个节点
	}
}

/************************************************************************************/

/*
 * function: get_decimal_point
 * description: get the decimal point character of the current locale
 */
ICACHE_FLASH_ATTR static unsigned char get_decimal_point(void) {
#ifdef ENABLE_LOCALES
	struct lconv *lconv = localeconv();
	return (unsigned char) lconv->decimal_point[0];
#else
	return '.';
#endif
}

/************************************************************************************/

/* 解析缓冲区的结构体 */
typedef struct {
	const unsigned char *content;
	size_t length;
	size_t offset;
	size_t depth; /* How deeply nested (in arrays/objects) is the input at the current offset. */
	internal_hooks hooks;	// 钩子
} parse_buffer;

/* check if the given size is left to read in a given parse buffer (starting with 1) */
#define can_read(buffer, size) ((buffer != NULL) && (((buffer)->offset + size) <= (buffer)->length))

/* check if the buffer can be accessed at the given index (starting with 0) */
#define can_access_at_index(buffer, index) ((buffer != NULL) && (((buffer)->offset + index) < (buffer)->length))
#define cannot_access_at_index(buffer, index) (!can_access_at_index(buffer, index))

/* get a pointer to the buffer at the position */
#define buffer_at_offset(buffer) ((buffer)->content + (buffer)->offset)

/*
 * function: parse_number
 * parameter: cJSON * const item - 输出的item
 *            parse_buffer * const input_buffer - 输入缓存区
 * return: cJSON_bool
 * description: Parse the input text to generate a number, and populate the result into item.
 *              解析输入的文本，生成一个数字，并把结果移到item
 */
ICACHE_FLASH_ATTR static cJSON_bool parse_number(cJSON * const item,
		parse_buffer * const input_buffer) {
	double number = 0;
	unsigned char *after_end = NULL;
	unsigned char number_c_string[64];
	unsigned char decimal_point = get_decimal_point();	// decimal_point 初始化
	size_t i = 0;

	// 检查输入的参数
	if ((input_buffer == NULL) || (input_buffer->content == NULL)) {
		return false;
	}

	/* copy the number into a temporary buffer and replace '.' with the decimal point
	 * of the current locale (for strtod)
	 * This also takes care of '\0' not necessarily being available for marking the end of the input */
	for (i = 0;
			(i < (sizeof(number_c_string) - 1))
					&& can_access_at_index(input_buffer, i); i++) {
		switch (buffer_at_offset(input_buffer)[i]) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case '+':
		case '-':
		case 'e':
		case 'E':
			number_c_string[i] = buffer_at_offset(input_buffer)[i];
			break;
		// 有小数点
		case '.':
			number_c_string[i] = decimal_point;
			break;

		default:
			goto loop_end;
		}
	}

	loop_end: number_c_string[i] = '\0';	// 设置字符串末尾

#ifndef CJSON_FOR_ESP8266
	/*
	 * strtod：将字符串转换成浮点数，有关头文件是#include <stdlib.h>
	 */
	number = strtod((const char*)number_c_string, (char**)&after_end);
	if (number_c_string == after_end)
	{
		return false; /* parse_error */
	}
#else
	number = atoi(number_c_string);
	if (0 == i) {
		return false;
	}
#endif

	// 同时设置 item->valuedouble 和 item->valueint

	// 设置浮点数
	item->valuedouble = number;

	// 设置整型，并防止溢出
	/* use saturation in case of overflow */
	if (number >= INT_MAX) {
		item->valueint = INT_MAX;
	} else if (number <= INT_MIN) {
		item->valueint = INT_MIN;
	} else {
		item->valueint = (int) number;
	}

	// 以上代码相当于 cJSON_SetNumberHelper 函数

	// 设置类型
	item->type = cJSON_Number;

#ifndef CJSON_FOR_ESP8266
	// 缓存区指针移动
	input_buffer->offset += (size_t)(after_end - number_c_string);
#else
	input_buffer->offset += (size_t) i;
#endif
	return true;
}

/*
 * function: cJSON_SetNumberHelper
 * description: 设置数字辅助函数
 */
/* don't ask me, but the original cJSON_SetNumberValue returns an integer or double */
CJSON_PUBLIC(double) cJSON_SetNumberHelper(cJSON *object, double number) {
	if (number >= INT_MAX) {
		object->valueint = INT_MAX;
	} else if (number <= INT_MIN) {
		object->valueint = INT_MIN;
	} else {
		object->valueint = (int) number;
	}

	return object->valuedouble = number;
}

/* 打印缓存结构体 */
typedef struct {
	unsigned char *buffer;
	size_t length;
	size_t offset;
	size_t depth; /* current nesting depth (for formatted printing) */
	cJSON_bool noalloc;
	cJSON_bool format; /* is this print a formatted print */
	internal_hooks hooks;
} printbuffer;

/*
 * function: ensure
 * parameter: printbuffer * const p -
 *            size_t needed -
 * return: unsigned char*
 * description: realloc printbuffer if necessary to have at least "needed" bytes more
 */
ICACHE_FLASH_ATTR static unsigned char* ensure(printbuffer * const p,
		size_t needed) {
	unsigned char *newbuffer = NULL;
	size_t newsize = 0;

	if ((p == NULL) || (p->buffer == NULL)) {
		return NULL;
	}

	if ((p->length > 0) && (p->offset >= p->length)) {
		/* make sure that offset is valid */
		return NULL;
	}

	if (needed > INT_MAX) {
		/* sizes bigger than INT_MAX are currently not supported */
		return NULL;
	}

	needed += p->offset + 1;
	if (needed <= p->length) {
		return p->buffer + p->offset;
	}

	if (p->noalloc) {
		return NULL;
	}

	/* calculate new buffer size */
	if (needed > (INT_MAX / 2)) {
		/* overflow of int, use INT_MAX if possible */
		if (needed <= INT_MAX) {
			newsize = INT_MAX;
		} else {
			return NULL;
		}
	} else {
		newsize = needed * 2;
	}

	if (p->hooks.reallocate != NULL) {
		/* reallocate with realloc if available */
		newbuffer = (unsigned char*) p->hooks.reallocate(p->buffer, newsize);
		if (newbuffer == NULL) {
			p->hooks.deallocate(p->buffer);
			p->length = 0;
			p->buffer = NULL;

			return NULL;
		}
	} else {
		/* otherwise reallocate manually */
		newbuffer = (unsigned char*) p->hooks.allocate(newsize);
		if (!newbuffer) {
			p->hooks.deallocate(p->buffer);
			p->length = 0;
			p->buffer = NULL;

			return NULL;
		}
		if (newbuffer) {
			os_memcpy(newbuffer, p->buffer, p->offset + 1);
		}
		p->hooks.deallocate(p->buffer);
	}
	p->length = newsize;
	p->buffer = newbuffer;

	return newbuffer + p->offset;
}

/* calculate the new length of the string in a printbuffer and update the offset */
ICACHE_FLASH_ATTR static void update_offset(printbuffer * const buffer) {
	const unsigned char *buffer_pointer = NULL;
	if ((buffer == NULL) || (buffer->buffer == NULL)) {
		return;
	}
	buffer_pointer = buffer->buffer + buffer->offset;

	buffer->offset += os_strlen((const char*) buffer_pointer);
}

	/*
 * function: print_number
 * parameter:
 * return: cJSON_bool -
 * description: Render the number nicely from the given item into a string.
 *              从给出的item友好地渲染，并放进string
 */
ICACHE_FLASH_ATTR static cJSON_bool print_number(const cJSON * const item,
		printbuffer * const output_buffer) {
	unsigned char *output_pointer = NULL;
	double d = item->valuedouble;
	int length = 0;
	size_t i = 0;
	unsigned char number_buffer[26]; /* temporary buffer to print the number into */
	unsigned char decimal_point = get_decimal_point();
	double test;

	// 检查参数
	if (output_buffer == NULL) {
		return false;
	}

	/* This checks for NaN and Infinity */
	if ((d * 0) != 0) {
		length = os_sprintf((char*) number_buffer, "null");
	} else {
#ifndef CJSON_FOR_ESP8266
		/* Try 15 decimal places of precision to avoid nonsignificant nonzero digits */
		length = os_sprintf((char*)number_buffer, "%1.15g", d);

		/* Check whether the original double can be recovered */
		if ((sscanf((char*)number_buffer, "%lg", &test) != 1) || ((double)test != d))
		{
			/* If not, print with 17 decimal places of precision */
			length = os_sprintf((char*)number_buffer, "%1.17g", d);
		}
#else
		// ESP8266 不支持 float 和 double
		length = os_sprintf((char*) number_buffer, "%d", item->valueint);
#endif
	}

	/* os_sprintf failed or buffer overrun occured */
	if ((length < 0) || (length > (int) (sizeof(number_buffer) - 1))) {
		return false;
	}

	/* reserve appropriate space in the output */
	output_pointer = ensure(output_buffer, (size_t) length);
	if (output_pointer == NULL) {
		return false;
	}

	/* copy the printed number to the output and replace locale
	 * dependent decimal point with '.' */
	for (i = 0; i < ((size_t) length); i++) {
		if (number_buffer[i] == decimal_point) {
			output_pointer[i] = '.';
			continue;
		}

		output_pointer[i] = number_buffer[i];
	}
	output_pointer[i] = '\0';

	output_buffer->offset += (size_t) length;

	return true;
}

/*
 * function: parse_hex4
 * description: parse 4 digit hexadecimal number
 */
ICACHE_FLASH_ATTR static unsigned parse_hex4(const unsigned char * const input) {
	unsigned int h = 0;
	size_t i = 0;

	for (i = 0; i < 4; i++) {
		/* parse digit */
		if ((input[i] >= '0') && (input[i] <= '9')) {
			h += (unsigned int) input[i] - '0';
		} else if ((input[i] >= 'A') && (input[i] <= 'F')) {
			h += (unsigned int) 10 + input[i] - 'A';
		} else if ((input[i] >= 'a') && (input[i] <= 'f')) {
			h += (unsigned int) 10 + input[i] - 'a';
		} else /* invalid */
		{
			return 0;
		}

		if (i < 3) {
			/* shift left to make place for the next nibble */
			h = h << 4;
		}
	}

	return h;
}

/*
 * function: utf16_literal_to_utf8
 * description: converts a UTF-16 literal to UTF-8
 *              A literal can be one or two sequences of the form \uXXXX
 */
ICACHE_FLASH_ATTR static unsigned char utf16_literal_to_utf8(
		const unsigned char * const input_pointer,
		const unsigned char * const input_end, unsigned char **output_pointer) {
	long unsigned int codepoint = 0;
	unsigned int first_code = 0;
	const unsigned char *first_sequence = input_pointer;
	unsigned char utf8_length = 0;
	unsigned char utf8_position = 0;
	unsigned char sequence_length = 0;
	unsigned char first_byte_mark = 0;

	if ((input_end - first_sequence) < 6) {
		/* input ends unexpectedly */
		goto fail;
	}

	/* get the first utf16 sequence */
	first_code = parse_hex4(first_sequence + 2);

	/* check that the code is valid */
	if (((first_code >= 0xDC00) && (first_code <= 0xDFFF))) {
		goto fail;
	}

	/* UTF16 surrogate pair */
	if ((first_code >= 0xD800) && (first_code <= 0xDBFF)) {
		const unsigned char *second_sequence = first_sequence + 6;
		unsigned int second_code = 0;
		sequence_length = 12; /* \uXXXX\uXXXX */

		if ((input_end - second_sequence) < 6) {
			/* input ends unexpectedly */
			goto fail;
		}

		if ((second_sequence[0] != '\\') || (second_sequence[1] != 'u')) {
			/* missing second half of the surrogate pair */
			goto fail;
		}

		/* get the second utf16 sequence */
		second_code = parse_hex4(second_sequence + 2);
		/* check that the code is valid */
		if ((second_code < 0xDC00) || (second_code > 0xDFFF)) {
			/* invalid second half of the surrogate pair */
			goto fail;
		}

		/* calculate the unicode codepoint from the surrogate pair */
		codepoint = 0x10000
				+ (((first_code & 0x3FF) << 10) | (second_code & 0x3FF));
	} else {
		sequence_length = 6; /* \uXXXX */
		codepoint = first_code;
	}

	/* encode as UTF-8
	 * takes at maximum 4 bytes to encode:
	 * 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
	if (codepoint < 0x80) {
		/* normal ascii, encoding 0xxxxxxx */
		utf8_length = 1;
	} else if (codepoint < 0x800) {
		/* two bytes, encoding 110xxxxx 10xxxxxx */
		utf8_length = 2;
		first_byte_mark = 0xC0; /* 11000000 */
	} else if (codepoint < 0x10000) {
		/* three bytes, encoding 1110xxxx 10xxxxxx 10xxxxxx */
		utf8_length = 3;
		first_byte_mark = 0xE0; /* 11100000 */
	} else if (codepoint <= 0x10FFFF) {
		/* four bytes, encoding 1110xxxx 10xxxxxx 10xxxxxx 10xxxxxx */
		utf8_length = 4;
		first_byte_mark = 0xF0; /* 11110000 */
	} else {
		/* invalid unicode codepoint */
		goto fail;
	}

	/* encode as utf8 */
	for (utf8_position = (unsigned char) (utf8_length - 1); utf8_position > 0;
			utf8_position--) {
		/* 10xxxxxx */
		(*output_pointer)[utf8_position] = (unsigned char) ((codepoint | 0x80)
				& 0xBF);
		codepoint >>= 6;
	}
	/* encode first byte */
	if (utf8_length > 1) {
		(*output_pointer)[0] = (unsigned char) ((codepoint | first_byte_mark)
				& 0xFF);
	} else {
		(*output_pointer)[0] = (unsigned char) (codepoint & 0x7F);
	}

	*output_pointer += utf8_length;

	return sequence_length;

	fail: return 0;
}

/*
 * function: parse_string
 * parameter: cJSON * const item - 输出的item
 *            parse_buffer * const input_buffer - 要解析的字符串缓冲区
 * return: cJSON_bool
 * description: Parse the input text into an unescaped cinput, and populate item.
 *              解析输入的文本，并放进
 */
ICACHE_FLASH_ATTR static cJSON_bool parse_string(cJSON * const item,
		parse_buffer * const input_buffer) {
	const unsigned char *input_pointer = buffer_at_offset(input_buffer) + 1;
	const unsigned char *input_end = buffer_at_offset(input_buffer) + 1;	// 初始化 input_end 指针
	unsigned char *output_pointer = NULL;
	unsigned char *output = NULL;

	/* not a string */
	// 输入字符串检查，如果第一个字符不是分号
	if (buffer_at_offset(input_buffer)[0] != '\"') {
		goto fail;
	}

	/*
	 * 示例："Jack (\"Bee\") Nimble" - 4+1+9+1+6+1=22个字节
	 * 减去转义字符，则只需要 20 字节的空间
	 */
	{
		/* calculate approximate size of the output (overestimate) */
		// 计算可能需要的输出空间
		size_t allocation_length = 0;
		size_t skipped_bytes = 0;		// 跳过的字节

		// 如果 input_end 没有扫描到 input_buffer的末尾，并且input不等于 \"
		while (((size_t) (input_end - input_buffer->content)
				< input_buffer->length) && (*input_end != '\"')) {
			/* is escape sequence */
			if (input_end[0] == '\\')	// 如果是转义字符
					{
				if ((size_t) (input_end + 1 - input_buffer->content)
						>= input_buffer->length) {
					/* prevent buffer overflow when last input character is a backslash */
					goto fail;
				}
				skipped_bytes++;		// 增加一个要跳过的字符
				input_end++;
			}
			input_end++;
		}
		if (((size_t) (input_end - input_buffer->content)
				>= input_buffer->length) || (*input_end != '\"')) {
			goto fail;
			/* string ended unexpectedly */
		}

		/* This is at most how much we need for the output */
		allocation_length =
				(size_t) (input_end - buffer_at_offset(input_buffer))
						- skipped_bytes;
		// 通过 hooks.allocate 获取内存
		output = (unsigned char*) input_buffer->hooks.allocate(
				allocation_length + sizeof(""));
		if (output == NULL) {
			goto fail;
			/* allocation failure */
		}
	}

	// 设置输出指针
	output_pointer = output;
	/* loop through the string literal */
	while (input_pointer < input_end) {
		// 如果不是转移字符，则直接拷贝
		if (*input_pointer != '\\') {
			*output_pointer++ = *input_pointer++;
		}
		/* escape sequence */
		// 转义序列
		else {
			unsigned char sequence_length = 2;
			if ((input_end - input_pointer) < 1) {
				goto fail;
			}

			switch (input_pointer[1]) {
			case 'b':
				*output_pointer++ = '\b';
				break;
			case 'f':
				*output_pointer++ = '\f';
				break;
			case 'n':
				*output_pointer++ = '\n';
				break;
			case 'r':
				*output_pointer++ = '\r';
				break;
			case 't':
				*output_pointer++ = '\t';
				break;
			case '\"':
			case '\\':
			case '/':
				*output_pointer++ = input_pointer[1];
				break;

				/* UTF-16 literal */
			case 'u':
				sequence_length = utf16_literal_to_utf8(input_pointer,
						input_end, &output_pointer);
				if (sequence_length == 0) {
					/* failed to convert UTF16-literal to UTF-8 */
					goto fail;
				}
				break;

			default:
				goto fail;
			}
			input_pointer += sequence_length;
		}
	}

	/* zero terminate the output */
	// 设置字符串末尾
	*output_pointer = '\0';

	// 设置类型
	item->type = cJSON_String;
	// 设置字符串值
	item->valuestring = (char*) output;

	input_buffer->offset = (size_t) (input_end - input_buffer->content);
	input_buffer->offset++;

	return true;

fail:
	 if (output != NULL) {
		input_buffer->hooks.deallocate(output);
	}

	if (input_pointer != NULL) {
		input_buffer->offset = (size_t) (input_pointer - input_buffer->content);
	}

	return false;
}

/*
 * function: print_string_ptr
 * parameter:
 * return: cJSON_bool
 * description: Render the cstring provided to an escaped version that can be printed.
 */
ICACHE_FLASH_ATTR static cJSON_bool print_string_ptr(
		const unsigned char * const input, printbuffer * const output_buffer) {
	const unsigned char *input_pointer = NULL;
	unsigned char *output = NULL;
	unsigned char *output_pointer = NULL;
	size_t output_length = 0;
	/* numbers of additional characters needed for escaping */
	size_t escape_characters = 0;

	if (output_buffer == NULL) {
		return false;
	}

	/* empty string */
	if (input == NULL) {
		output = ensure(output_buffer, sizeof("\"\""));
		if (output == NULL) {
			return false;
		}
		os_strcpy((char*) output, "\"\"");

		return true;
	}

	/* set "flag" to 1 if something needs to be escaped */
	for (input_pointer = input; *input_pointer; input_pointer++) {
		switch (*input_pointer) {
		case '\"':
		case '\\':
		case '\b':
		case '\f':
		case '\n':
		case '\r':
		case '\t':
			/* one character escape sequence */
			escape_characters++;
			break;
		default:
			if (*input_pointer < 32) {
				/* UTF-16 escape sequence uXXXX */
				escape_characters += 5;
			}
			break;
		}
	}
	output_length = (size_t) (input_pointer - input) + escape_characters;

	output = ensure(output_buffer, output_length + sizeof("\"\""));
	if (output == NULL) {
		return false;
	}

	/* no characters have to be escaped */
	if (escape_characters == 0) {
		output[0] = '\"';
		os_memcpy(output + 1, input, output_length);
		output[output_length + 1] = '\"';
		output[output_length + 2] = '\0';

		return true;
	}

	output[0] = '\"';
	output_pointer = output + 1;
	/* copy the string */
	for (input_pointer = input; *input_pointer != '\0';
			(void) input_pointer++, output_pointer++) {
		if ((*input_pointer > 31) && (*input_pointer != '\"')
				&& (*input_pointer != '\\')) {
			/* normal character, copy */
			*output_pointer = *input_pointer;
		} else {
			/* character needs to be escaped */
			*output_pointer++ = '\\';
			switch (*input_pointer) {
			case '\\':
				*output_pointer = '\\';
				break;
			case '\"':
				*output_pointer = '\"';
				break;
			case '\b':
				*output_pointer = 'b';
				break;
			case '\f':
				*output_pointer = 'f';
				break;
			case '\n':
				*output_pointer = 'n';
				break;
			case '\r':
				*output_pointer = 'r';
				break;
			case '\t':
				*output_pointer = 't';
				break;
			default:
				/* escape and print as unicode codepoint */
				os_sprintf((char*) output_pointer, "u%04x", *input_pointer);
				output_pointer += 4;
				break;
			}
		}
	}
	output[output_length + 1] = '\"';
	output[output_length + 2] = '\0';

	return true;
}

/*
 * function: print_string
 * parameter:
 * return: cJSON_bool
 * description: Invoke print_string_ptr (which is useful) on an item.
 */
ICACHE_FLASH_ATTR static cJSON_bool print_string(const cJSON * const item,
		printbuffer * const p) {
	return print_string_ptr((unsigned char*) item->valuestring, p);
}

/* Predeclare these prototypes. */
static cJSON_bool parse_value(cJSON * const item,
		parse_buffer * const input_buffer);
static cJSON_bool print_value(const cJSON * const item,
		printbuffer * const output_buffer);
static cJSON_bool parse_array(cJSON * const item,
		parse_buffer * const input_buffer);
static cJSON_bool print_array(const cJSON * const item,
		printbuffer * const output_buffer);
static cJSON_bool parse_object(cJSON * const item,
		parse_buffer * const input_buffer);
static cJSON_bool print_object(const cJSON * const item,
		printbuffer * const output_buffer);

/*
 * function: buffer_skip_whitespace
 * parameter: parse_buffer * const buffer
 * return: parse_buffer * -
 * description: Utility to jump whitespace and cr/lf
 *              跳过空格和'\r'、'\n'
 */
ICACHE_FLASH_ATTR static parse_buffer *buffer_skip_whitespace(
		parse_buffer * const buffer) {
	// 检查参数
	if ((buffer == NULL) || (buffer->content == NULL)) {
		return NULL;
	}

	while (can_access_at_index(buffer, 0) && (buffer_at_offset(buffer)[0] <= 32)) {
		buffer->offset++;
	}

	if (buffer->offset == buffer->length) {
		buffer->offset--;
	}

	return buffer;
}

/*
 * function: skip_utf8_bom
 * parameter: parse_buffer * const buffer
 * return: parse_buffer *
 * description: skip the UTF-8 BOM (byte order mark) if it is at the beginning of a buffer
 */
ICACHE_FLASH_ATTR static parse_buffer *skip_utf8_bom(
		parse_buffer * const buffer) {
	if ((buffer == NULL) || (buffer->content == NULL)
			|| (buffer->offset != 0)) {
		return NULL;
	}

	if (can_access_at_index(buffer, 4)
			&& (os_strncmp((const char*) buffer_at_offset(buffer),
					"\xEF\xBB\xBF", 3) == 0)) {
		buffer->offset += 3;
	}

	return buffer;
}

/*
 * function: cJSON_ParseWithOpts
 * parameter: const char *value - 锟街凤拷锟斤拷值
 *            const char **return_parse_end - output
 *            cJSON_bool require_null_terminated -
 * return: CJSON_PUBLIC(cJSON *)
 * description: Parse an object - create a new root, and populate.
 *               解析一个对象 - 构建一个新的root，并移过去
 */
CJSON_PUBLIC(cJSON *) cJSON_ParseWithOpts(const char *value, const char **return_parse_end, cJSON_bool require_null_terminated)
{
	parse_buffer buffer = {0, 0, 0, 0, {0, 0, 0}};
	cJSON *item = NULL;

	/* reset error position */
	global_error.json = NULL;
	global_error.position = 0;

	if (value == NULL)
	{
		goto fail;
	}

	buffer.content = (const unsigned char*)value;
	buffer.length = os_strlen((const char*)value) + sizeof("");
	buffer.offset = 0;
	buffer.hooks = global_hooks;

	item = cJSON_New_Item(&global_hooks);
	if (item == NULL) /* memory fail */
	{
		goto fail;
	}

	if (!parse_value(item, buffer_skip_whitespace(skip_utf8_bom(&buffer))))
	{
		/* parse failure. ep is set. */
		goto fail;
	}

	/* if we require null-terminated JSON without appended garbage, skip and then check for a null terminator */
	if (require_null_terminated)
	{
		buffer_skip_whitespace(&buffer);
		if ((buffer.offset >= buffer.length) || buffer_at_offset(&buffer)[0] != '\0')
		{
			goto fail;
		}
	}

	if (return_parse_end)
	{
		*return_parse_end = (const char*)buffer_at_offset(&buffer);
	}

	return item;

	// 错误处理
fail:
	if (item != NULL)
	{
		cJSON_Delete(item);
	}

	if (value != NULL)
	{
		error local_error;
		local_error.json = (const unsigned char*)value;
		local_error.position = 0;

		if (buffer.offset < buffer.length)
		{
			local_error.position = buffer.offset;
		}
		else if (buffer.length > 0)
		{
			local_error.position = buffer.length - 1;
		}

		if (return_parse_end != NULL)
		{
			*return_parse_end = (const char*)local_error.json + local_error.position;
		}

		global_error = local_error;
	}

	return NULL;
}

	/*
 * function: cJSON_Parse
 * parameter: const char *value - 字符串值
 * return: CJSON_PUBLIC(cJSON *)
 * description: Default options for cJSON_Parse
 *              字符串解释成cJSON函数
 */
CJSON_PUBLIC(cJSON *) cJSON_Parse(const char *value)
{
	return cJSON_ParseWithOpts(value, 0, 0);
}

// 比较大小
#define cjson_min(a, b) ((a < b) ? a : b)

	/*
 * function: print
 * parameter: const cJSON * const item - item
 *            cJSON_bool format - 是否格式化
 *            const internal_hooks * const hooks - 钩子函数
 * return: unsigned char *
 * description:
 */
ICACHE_FLASH_ATTR static unsigned char *print(const cJSON * const item,
		cJSON_bool format, const internal_hooks * const hooks) {
	printbuffer buffer[1];
	unsigned char *printed = NULL;

	os_memset(buffer, 0, sizeof(buffer));

	/* create buffer */
	buffer->buffer = (unsigned char*) hooks->allocate(256);
	buffer->format = format;
	buffer->hooks = *hooks;
	if (buffer->buffer == NULL) {
		goto fail;
	}

	/* print the value */
	if (!print_value(item, buffer)) {
		goto fail;
	}
	update_offset(buffer);

	/* check if reallocate is available */
	if (hooks->reallocate != NULL) {
		printed = (unsigned char*) hooks->reallocate(buffer->buffer,
				buffer->length);
		buffer->buffer = NULL;
		if (printed == NULL) {
			goto fail;
		}
	} else /* otherwise copy the JSON over to a new buffer */
	{
		printed = (unsigned char*) hooks->allocate(buffer->offset + 1);
		if (printed == NULL) {
			goto fail;
		}
		os_memcpy(printed, buffer->buffer,
				cjson_min(buffer->length, buffer->offset + 1));
		printed[buffer->offset] = '\0'; /* just to be sure */

		/* free the buffer */
		hooks->deallocate(buffer->buffer);
	}

	return printed;

	fail: if (buffer->buffer != NULL) {
		hooks->deallocate(buffer->buffer);
	}

	if (printed != NULL) {
		hooks->deallocate(printed);
	}

	return NULL;
}

/*
 * function: cJSON_Print
 * parameter: const cJSON *item
 * return: CJSON_PUBLIC(char *)
 * description: Render a cJSON item/entity/structure to text.
 */
CJSON_PUBLIC(char *) cJSON_Print(const cJSON *item)
{
	return (char*)print(item, true, &global_hooks);
}

CJSON_PUBLIC(char *) cJSON_PrintUnformatted(const cJSON *item)
{
	return (char*)print(item, false, &global_hooks);
}

CJSON_PUBLIC(char *) cJSON_PrintBuffered(const cJSON *item, int prebuffer, cJSON_bool fmt)
{
	printbuffer p = {0, 0, 0, 0, 0, 0, {0, 0, 0}};

	if (prebuffer < 0)
	{
		return NULL;
	}

	p.buffer = (unsigned char*)global_hooks.allocate((size_t)prebuffer);
	if (!p.buffer)
	{
		return NULL;
	}

	p.length = (size_t)prebuffer;
	p.offset = 0;
	p.noalloc = false;
	p.format = fmt;
	p.hooks = global_hooks;

	if (!print_value(item, &p))
	{
		global_hooks.deallocate(p.buffer);
		return NULL;
	}

	return (char*)p.buffer;
}

CJSON_PUBLIC(cJSON_bool) cJSON_PrintPreallocated(cJSON *item, char *buf,
		const int len, const cJSON_bool fmt) {
	printbuffer p = { 0, 0, 0, 0, 0, 0, { 0, 0, 0 } };

	if ((len < 0) || (buf == NULL)) {
		return false;
	}

	p.buffer = (unsigned char*) buf;
	p.length = (size_t) len;
	p.offset = 0;
	p.noalloc = true;
	p.format = fmt;
	p.hooks = global_hooks;

	return print_value(item, &p);
}

	/*
 * function: parse_value
 * parameter: cJSON * const item -
 *            parse_buffer * const input_buffer - 解析缓冲结构体
 * return: cJSON_bool
 * description: Parser core - when encountering text, process appropriately.
 *              解析核心函数
 */
static cJSON_bool parse_value(cJSON * const item,
		parse_buffer * const input_buffer) {
	// 锟斤拷锟斤拷锟斤拷锟�
	if ((input_buffer == NULL) || (input_buffer->content == NULL)) {
		return false; /* no input */
	}

	/* parse the different types of values */
	/* null */
	if (can_read(input_buffer, 4)
			&& (os_strncmp((const char*) buffer_at_offset(input_buffer), "null",
					4) == 0)) {
		item->type = cJSON_NULL;
		input_buffer->offset += 4;	// 跳过 'null' 字符串的长度
		return true;
	}
	/* false */
	if (can_read(input_buffer, 5)
			&& (os_strncmp((const char*) buffer_at_offset(input_buffer),
					"false", 5) == 0)) {
		item->type = cJSON_False;
		input_buffer->offset += 5;	// 跳过 'false' 字符串的长度
		return true;
	}
	/* true */
	if (can_read(input_buffer, 4)
			&& (os_strncmp((const char*) buffer_at_offset(input_buffer), "true",
					4) == 0)) {
		item->type = cJSON_True;
		item->valueint = 1;
		input_buffer->offset += 4;	// 跳过 'true' 字符串的长度
		return true;
	}
	/* string */
	// input_buffer[0]可读 并且 第一个字符是分号
	if (can_access_at_index(input_buffer, 0)
			&& (buffer_at_offset(input_buffer)[0] == '\"')) {
		return parse_string(item, input_buffer);
	}
	/* number */
	// input_buffer[0]可读 并且 [第一个字符 是负号（-） 或者 是大于'0'且小于'9']
	if (can_access_at_index(input_buffer, 0)
			&& ((buffer_at_offset(input_buffer)[0] == '-')
					|| ((buffer_at_offset(input_buffer)[0] >= '0')
							&& (buffer_at_offset(input_buffer)[0] <= '9')))) {
		return parse_number(item, input_buffer);
	}
	/* array */
	// input_buffer[0]可读 并且 第一个字符是'['
	if (can_access_at_index(input_buffer, 0)
			&& (buffer_at_offset(input_buffer)[0] == '[')) {
		return parse_array(item, input_buffer);
	}
	/* object */
	// input_buffer[0]可读 并且 第一个字符是'{'
	if (can_access_at_index(input_buffer, 0)
			&& (buffer_at_offset(input_buffer)[0] == '{')) {
		return parse_object(item, input_buffer);
	}

	return false;
}

	/*
 * function: print_value
 * parameter:const cJSON * const item -
 *                 printbuffer * const output_buffer -
 * return: cJSON_bool
 * description: Render a value to text.
 *              渲染value到文本
 */
ICACHE_FLASH_ATTR static cJSON_bool print_value(const cJSON * const item,
		printbuffer * const output_buffer) {
	unsigned char *output = NULL;

	if ((item == NULL) || (output_buffer == NULL)) {
		return false;
	}

	// 选择类型
	switch ((item->type) & 0xFF) {
	case cJSON_NULL:
		output = ensure(output_buffer, 5);
		if (output == NULL) {
			return false;
		}
		os_strcpy((char*) output, "null");
		return true;

	case cJSON_False:
		output = ensure(output_buffer, 6);
		if (output == NULL) {
			return false;
		}
		os_strcpy((char*) output, "false");
		return true;

	case cJSON_True:
		output = ensure(output_buffer, 5);
		if (output == NULL) {
			return false;
		}
		os_strcpy((char*) output, "true");
		return true;

	case cJSON_Number:
		return print_number(item, output_buffer);

	case cJSON_Raw: {
		size_t raw_length = 0;
		if (item->valuestring == NULL) {
			if (!output_buffer->noalloc) {
				output_buffer->hooks.deallocate(output_buffer->buffer);
			}
			return false;
		}

		raw_length = os_strlen(item->valuestring) + sizeof("");
		output = ensure(output_buffer, raw_length);
		if (output == NULL) {
			return false;
		}
		os_memcpy(output, item->valuestring, raw_length);
		return true;
	}

	case cJSON_String:
		return print_string(item, output_buffer);

	case cJSON_Array:
		return print_array(item, output_buffer);

	case cJSON_Object:
		return print_object(item, output_buffer);

	default:
		return false;
	}
}

	/*
 * function: parse_array
 * parameter: cJSON * const item - 输出的item
 *            parse_buffer * const input_buffer - 输入的缓冲
 * return: cJSON_bool
 * description: Build an array from input text.
 */
ICACHE_FLASH_ATTR static cJSON_bool parse_array(cJSON * const item,
		parse_buffer * const input_buffer) {
	cJSON *head = NULL; /* head of the linked list */
	cJSON *current_item = NULL;

	// 检查输入的缓冲
	// 如果深度 >= CJSON_NESTING_LIMIT（默认1000） ，则出错
	if (input_buffer->depth >= CJSON_NESTING_LIMIT) {
		return false; /* to deeply nested */
	}
	input_buffer->depth++;

	// 检查第一个字符是不是 '['
	if (buffer_at_offset(input_buffer)[0] != '[') {
		/* not an array */
		goto fail;
	}

	input_buffer->offset++;
	buffer_skip_whitespace(input_buffer);
	if (can_access_at_index(input_buffer, 0)
			&& (buffer_at_offset(input_buffer)[0] == ']')) {
		/* empty array */
		goto success;
	}

	/* check if we skipped to the end of the buffer */
	if (cannot_access_at_index(input_buffer, 0)) {
		input_buffer->offset--;
		goto fail;
	}

	/* step back to character in front of the first element */
	input_buffer->offset--;
	/* loop through the comma separated array elements */
	do {
		/* allocate next item */
		cJSON *new_item = cJSON_New_Item(&(input_buffer->hooks));
		if (new_item == NULL) {
			goto fail;
			/* allocation failure */
		}

		/* attach next item to list */
		if (head == NULL) {
			/* start the linked list */
			current_item = head = new_item;
		} else {
			/* add to the end and advance */
			current_item->next = new_item;
			new_item->prev = current_item;
			current_item = new_item;
		}

		/* parse next value */
		input_buffer->offset++;
		buffer_skip_whitespace(input_buffer);
		if (!parse_value(current_item, input_buffer)) {
			goto fail;
			/* failed to parse value */
		}
		buffer_skip_whitespace(input_buffer);
	} while (can_access_at_index(input_buffer, 0)
			&& (buffer_at_offset(input_buffer)[0] == ','));

	if (cannot_access_at_index(input_buffer, 0)
			|| buffer_at_offset(input_buffer)[0] != ']') {
		goto fail;
		/* expected end of array */
	}

	success: input_buffer->depth--;

	// 设置类型
	item->type = cJSON_Array;
	item->child = head;

	input_buffer->offset++;

	return true;

fail:
	if (head != NULL) {
		cJSON_Delete(head);
	}

	return false;
}

/*
 * function: print_array
 * parameter: const cJSON * const item -
              printbuffer * const output_buffer
 * description: Render an array to text
 */
ICACHE_FLASH_ATTR static cJSON_bool print_array(const cJSON * const item,
		printbuffer * const output_buffer) {
	unsigned char *output_pointer = NULL;
	size_t length = 0;
	cJSON *current_element = item->child;

	if (output_buffer == NULL) {
		return false;
	}

	/* Compose the output array. */
	/* opening square bracket */
	output_pointer = ensure(output_buffer, 1);
	if (output_pointer == NULL) {
		return false;
	}

	*output_pointer = '[';
	output_buffer->offset++;
	output_buffer->depth++;

	while (current_element != NULL) {
		if (!print_value(current_element, output_buffer)) {
			return false;
		}
		update_offset(output_buffer);
		if (current_element->next) {
			length = (size_t) (output_buffer->format ? 2 : 1);
			output_pointer = ensure(output_buffer, length + 1);
			if (output_pointer == NULL) {
				return false;
			}
			*output_pointer++ = ',';
			if (output_buffer->format) {
				*output_pointer++ = ' ';
			}
			*output_pointer = '\0';
			output_buffer->offset += length;
		}
		current_element = current_element->next;
	}

	output_pointer = ensure(output_buffer, 2);
	if (output_pointer == NULL) {
		return false;
	}
	*output_pointer++ = ']';
	*output_pointer = '\0';
	output_buffer->depth--;

	return true;
}

/*
 * function: parse_object
 * parameter: cJSON * const item -
 *            parse_buffer * const input_buffer
 * return: cJSON_bool
 * description: Build an object from the text.
 */
ICACHE_FLASH_ATTR static cJSON_bool parse_object(cJSON * const item,
		parse_buffer * const input_buffer) {
	cJSON *head = NULL; /* linked list head */
	cJSON *current_item = NULL;

	if (input_buffer->depth >= CJSON_NESTING_LIMIT) {
		return false; /* to deeply nested */
	}
	input_buffer->depth++;

	if (cannot_access_at_index(input_buffer, 0)
			|| (buffer_at_offset(input_buffer)[0] != '{')) {
		goto fail;
		/* not an object */
	}

	input_buffer->offset++;
	buffer_skip_whitespace(input_buffer);
	if (can_access_at_index(input_buffer, 0)
			&& (buffer_at_offset(input_buffer)[0] == '}')) {
		goto success;
		/* empty object */
	}

	/* check if we skipped to the end of the buffer */
	if (cannot_access_at_index(input_buffer, 0)) {
		input_buffer->offset--;
		goto fail;
	}

	/* step back to character in front of the first element */
	input_buffer->offset--;
	/* loop through the comma separated array elements */
	do {
		/* allocate next item */
		cJSON *new_item = cJSON_New_Item(&(input_buffer->hooks));
		if (new_item == NULL) {
			goto fail;
			/* allocation failure */
		}

		/* attach next item to list */
		if (head == NULL) {
			/* start the linked list */
			current_item = head = new_item;
		} else {
			/* add to the end and advance */
			current_item->next = new_item;
			new_item->prev = current_item;
			current_item = new_item;
		}

		/* parse the name of the child */
		input_buffer->offset++;
		buffer_skip_whitespace(input_buffer);
		if (!parse_string(current_item, input_buffer)) {
			goto fail;
			/* faile to parse name */
		}
		buffer_skip_whitespace(input_buffer);

		/* swap valuestring and string, because we parsed the name */
		current_item->string = current_item->valuestring;
		current_item->valuestring = NULL;

		if (cannot_access_at_index(input_buffer, 0)
				|| (buffer_at_offset(input_buffer)[0] != ':')) {
			goto fail;
			/* invalid object */
		}

		/* parse the value */
		input_buffer->offset++;
		buffer_skip_whitespace(input_buffer);
		if (!parse_value(current_item, input_buffer)) {
			goto fail;
			/* failed to parse value */
		}
		buffer_skip_whitespace(input_buffer);
	} while (can_access_at_index(input_buffer, 0)
			&& (buffer_at_offset(input_buffer)[0] == ','));

	if (cannot_access_at_index(input_buffer, 0)
			|| (buffer_at_offset(input_buffer)[0] != '}')) {
		goto fail;
		/* expected end of object */
	}

	success: input_buffer->depth--;

	item->type = cJSON_Object;
	item->child = head;

	input_buffer->offset++;
	return true;

	fail: if (head != NULL) {
		cJSON_Delete(head);
	}

	return false;
}

/*
 * function: print_object
 * description: Render an object to text.
 */
ICACHE_FLASH_ATTR static cJSON_bool print_object(const cJSON * const item,
		printbuffer * const output_buffer) {
	unsigned char *output_pointer = NULL;
	size_t length = 0;
	cJSON *current_item = item->child;

	if (output_buffer == NULL) {
		return false;
	}

	/* Compose the output: */
	length = (size_t) (output_buffer->format ? 2 : 1); /* fmt: {\n */
	output_pointer = ensure(output_buffer, length + 1);
	if (output_pointer == NULL) {
		return false;
	}

	*output_pointer++ = '{';
	output_buffer->depth++;
	if (output_buffer->format) {
		*output_pointer++ = '\n';
	}
	output_buffer->offset += length;

	while (current_item) {
		if (output_buffer->format) {
			size_t i;
			output_pointer = ensure(output_buffer, output_buffer->depth);
			if (output_pointer == NULL) {
				return false;
			}
			for (i = 0; i < output_buffer->depth; i++) {
				*output_pointer++ = '\t';
			}
			output_buffer->offset += output_buffer->depth;
		}

		/* print key */
		if (!print_string_ptr((unsigned char*) current_item->string,
				output_buffer)) {
			return false;
		}
		update_offset(output_buffer);

		length = (size_t) (output_buffer->format ? 2 : 1);
		output_pointer = ensure(output_buffer, length);
		if (output_pointer == NULL) {
			return false;
		}
		*output_pointer++ = ':';
		if (output_buffer->format) {
			*output_pointer++ = '\t';
		}
		output_buffer->offset += length;

		/* print value */
		if (!print_value(current_item, output_buffer)) {
			return false;
		}
		update_offset(output_buffer);

		/* print comma if not last */
		length = (size_t) ((output_buffer->format ? 1 : 0)
				+ (current_item->next ? 1 : 0));
		output_pointer = ensure(output_buffer, length + 1);
		if (output_pointer == NULL) {
			return false;
		}
		if (current_item->next) {
			*output_pointer++ = ',';
		}

		if (output_buffer->format) {
			*output_pointer++ = '\n';
		}
		*output_pointer = '\0';
		output_buffer->offset += length;

		current_item = current_item->next;
	}

	output_pointer = ensure(output_buffer,
			output_buffer->format ? (output_buffer->depth + 1) : 2);
	if (output_pointer == NULL) {
		return false;
	}
	if (output_buffer->format) {
		size_t i;
		for (i = 0; i < (output_buffer->depth - 1); i++) {
			*output_pointer++ = '\t';
		}
	}
	*output_pointer++ = '}';
	*output_pointer = '\0';
	output_buffer->depth--;

	return true;
}

/* Get Array size/item / object item. */
CJSON_PUBLIC(int) cJSON_GetArraySize(const cJSON *array) {
	cJSON *child = NULL;
	size_t size = 0;

	if (array == NULL) {
		return 0;
	}

	child = array->child;

	while (child != NULL) {
		size++;
		child = child->next;
	}

	/* FIXME: Can overflow here. Cannot be fixed without breaking the API */

	return (int) size;
}

/*
 * function: get_array_item
 */
ICACHE_FLASH_ATTR static cJSON* get_array_item(const cJSON *array, size_t index) {
	cJSON *current_child = NULL;

	if (array == NULL) {
		return NULL;
	}

	current_child = array->child;
	while ((current_child != NULL) && (index > 0)) {
		index--;
		current_child = current_child->next;
	}

	return current_child;
}

CJSON_PUBLIC(cJSON *) cJSON_GetArrayItem(const cJSON *array, int index)
{
	if (index < 0)
	{
		return NULL;
	}

	return get_array_item(array, (size_t)index);
}

	/*
 * function: get_object_item
 * parameter: const cJSON * const object -
 *            const char * const name -
 *            const cJSON_bool case_sensitive - 是否区分大小写
 * description:
 */
ICACHE_FLASH_ATTR static cJSON *get_object_item(const cJSON * const object,
		const char * const name, const cJSON_bool case_sensitive) {
	cJSON *current_element = NULL;

	// 检查参数
	if ((object == NULL) || (name == NULL)) {
		return NULL;
	}

	// 当前元素
	current_element = object->child;
	if (case_sensitive)	// 是否区分大小写
	{	// 遍历查找
		while ((current_element != NULL)
				&& (strcmp(name, current_element->string) != 0)) {
			current_element = current_element->next;
		}
	} else {	// 遍历查找
		while ((current_element != NULL)
				&& (case_insensitive_strcmp((const unsigned char*) name,
						(const unsigned char*) (current_element->string)) != 0)) {
			current_element = current_element->next;
		}
	}

	return current_element;
}

/*
 * function: cJSON_GetObjectItem
 * description:
 */
CJSON_PUBLIC(cJSON *) cJSON_GetObjectItem(const cJSON * const object, const char * const string)
{
	return get_object_item(object, string, false);
}

/*
 * function: cJSON_GetObjectItemCaseSensitive
 * description:
 */
CJSON_PUBLIC(cJSON *) cJSON_GetObjectItemCaseSensitive(const cJSON * const object, const char * const string)
{
	return get_object_item(object, string, true);
}

/*
 * function: cJSON_HasObjectItem
 * parameter: const cJSON *object - 一锟斤拷cJSON锟斤拷锟斤拷
 *            const char *string - JSON锟斤拷key
 * return: CJSON_PUBLIC(cJSON_bool)
 * description: 锟角凤拷锟斤拷某锟斤拷Key锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
 */
CJSON_PUBLIC(cJSON_bool) cJSON_HasObjectItem(const cJSON *object,
		const char *string) {
	return cJSON_GetObjectItem(object, string) ? 1 : 0;
}

/*
 * function: suffix_object
 * parameter: cJSON *prev -
 *            cJSON *item -
 * return: void
 * description: Utility for array list handling.
 *              锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
 */
ICACHE_FLASH_ATTR static void suffix_object(cJSON *prev, cJSON *item) {
	prev->next = item;	// 前一锟斤拷锟节碉拷锟斤拷锟斤拷next
	item->prev = prev;	// 锟斤拷锟斤拷前锟节碉拷锟斤拷锟斤拷prev
}

/*
 * function: create_reference
 * parameter: const cJSON *item -
 *            const internal_hooks * const hooks - 锟斤拷锟接猴拷锟斤拷
 * return: cJSON *
 * description: Utility for handling references.
 *              锟斤拷锟斤拷一锟斤拷cJSON锟斤拷锟斤拷锟斤拷
 */
ICACHE_FLASH_ATTR static cJSON *create_reference(const cJSON *item,
		const internal_hooks * const hooks) {
	cJSON *reference = NULL;
	if (item == NULL) {
		return NULL;
	}

	reference = cJSON_New_Item(hooks);
	if (reference == NULL) {
		return NULL;
	}

	os_memcpy(reference, item, sizeof(cJSON));
	reference->string = NULL;
	reference->type |= cJSON_IsReference;
	reference->next = reference->prev = NULL;
	return reference;
}

/**********************************************************************/
// cJSON_AddItemToXXX function
/*
 * function: cJSON_AddItemToArray
 * parameter: cJSON *array -
 *            cJSON *item
 * return: CJSON_PUBLIC(void)
 * description: Add item to array/object.
 *              锟斤拷锟节革拷array锟斤拷object锟斤拷锟絠tem锟斤拷锟斤拷cJSON锟斤拷obeject锟斤拷array锟斤拷一锟斤拷锟斤拷锟斤拷
 */
CJSON_PUBLIC(void) cJSON_AddItemToArray(cJSON *array, cJSON *item) {
	cJSON *child = NULL;

	// 锟斤拷锟斤拷锟斤拷
	if ((item == NULL) || (array == NULL)) {
		return;
	}

	child = array->child;

	if (child == NULL) {
		/* list is empty, start new one */
		array->child = item;
	} else {
		/* append to the end */
		while (child->next) {
			child = child->next;
		}
		suffix_object(child, item);
	}
}

CJSON_PUBLIC(void) cJSON_AddItemToObject(cJSON *object, const char *string,
		cJSON *item) {
	if (item == NULL) {
		return;
	}

	/* call cJSON_AddItemToObjectCS for code reuse */
	cJSON_AddItemToObjectCS(object,
			(char*) cJSON_strdup((const unsigned char*) string, &global_hooks),
			item);
	/* remove cJSON_StringIsConst flag */
	item->type &= ~cJSON_StringIsConst;
}

/**********************************************************************/

#if defined(__clang__) || (defined(__GNUC__)  && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ > 5))))
#pragma GCC diagnostic push
#endif
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif

/*
 * function: cJSON_AddItemToObjectCS
 * parameter: cJSON *object - 要锟斤拷咏锟斤拷锟給bject
 *            const char *string - JSON锟侥硷拷锟斤拷锟斤拷
 *            cJSON *item - 锟斤拷拥锟絠tem
 * description: Add an item to an object with constant string as key
 */
CJSON_PUBLIC(void) cJSON_AddItemToObjectCS(cJSON *object, const char *string,
		cJSON *item) {
	// 锟斤拷锟斤拷锟斤拷
	if ((item == NULL) || (string == NULL)) {
		return;
	}

	if (!(item->type & cJSON_StringIsConst) && item->string) {
		global_hooks.deallocate(item->string);
	}

	item->string = (char*) string;
	item->type |= cJSON_StringIsConst;

	cJSON_AddItemToArray(object, item); // 锟斤拷为锟斤拷锟斤拷锟斤拷咏锟給beject
}

#if defined(__clang__) || (defined(__GNUC__)  && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ > 5))))
#pragma GCC diagnostic pop
#endif

CJSON_PUBLIC(void) cJSON_AddItemReferenceToArray(cJSON *array, cJSON *item) {
	if (array == NULL) {
		return;
	}

	cJSON_AddItemToArray(array, create_reference(item, &global_hooks));
}

CJSON_PUBLIC(void) cJSON_AddItemReferenceToObject(cJSON *object,
		const char *string, cJSON *item) {
	if ((object == NULL) || (string == NULL)) {
		return;
	}

	cJSON_AddItemToObject(object, string,
			create_reference(item, &global_hooks));
}

CJSON_PUBLIC(cJSON *) cJSON_DetachItemViaPointer(cJSON *parent, cJSON * const item)
{
	if ((parent == NULL) || (item == NULL))
	{
		return NULL;
	}

	if (item->prev != NULL)
	{
		/* not the first element */
		item->prev->next = item->next;
	}
	if (item->next != NULL)
	{
		/* not the last element */
		item->next->prev = item->prev;
	}

	if (item == parent->child)
	{
		/* first element */
		parent->child = item->next;
	}
	/* make sure the detached item doesn't point anywhere anymore */
	item->prev = NULL;
	item->next = NULL;

	return item;
}

CJSON_PUBLIC(cJSON *) cJSON_DetachItemFromArray(cJSON *array, int which)
{
	if (which < 0)
	{
		return NULL;
	}

	return cJSON_DetachItemViaPointer(array, get_array_item(array, (size_t)which));
}

CJSON_PUBLIC(void) cJSON_DeleteItemFromArray(cJSON *array, int which) {
	cJSON_Delete(cJSON_DetachItemFromArray(array, which));
}

CJSON_PUBLIC(cJSON *) cJSON_DetachItemFromObject(cJSON *object, const char *string)
{
	cJSON *to_detach = cJSON_GetObjectItem(object, string);

	return cJSON_DetachItemViaPointer(object, to_detach);
}

CJSON_PUBLIC(cJSON *) cJSON_DetachItemFromObjectCaseSensitive(cJSON *object, const char *string)
{
	cJSON *to_detach = cJSON_GetObjectItemCaseSensitive(object, string);

	return cJSON_DetachItemViaPointer(object, to_detach);
}

CJSON_PUBLIC(void) cJSON_DeleteItemFromObject(cJSON *object, const char *string) {
	cJSON_Delete(cJSON_DetachItemFromObject(object, string));
}

CJSON_PUBLIC(void) cJSON_DeleteItemFromObjectCaseSensitive(cJSON *object,
		const char *string) {
	cJSON_Delete(cJSON_DetachItemFromObjectCaseSensitive(object, string));
}

/* Replace array/object items with new ones. */
CJSON_PUBLIC(void) cJSON_InsertItemInArray(cJSON *array, int which,
		cJSON *newitem) {
	cJSON *after_inserted = NULL;

	if (which < 0) {
		return;
	}

	after_inserted = get_array_item(array, (size_t) which);
	if (after_inserted == NULL) {
		cJSON_AddItemToArray(array, newitem);
		return;
	}

	newitem->next = after_inserted;
	newitem->prev = after_inserted->prev;
	after_inserted->prev = newitem;
	if (after_inserted == array->child) {
		array->child = newitem;
	} else {
		newitem->prev->next = newitem;
	}
}

CJSON_PUBLIC(cJSON_bool) cJSON_ReplaceItemViaPointer(cJSON * const parent,
		cJSON * const item, cJSON * replacement) {
	if ((parent == NULL) || (replacement == NULL) || (item == NULL)) {
		return false;
	}

	if (replacement == item) {
		return true;
	}

	replacement->next = item->next;
	replacement->prev = item->prev;

	if (replacement->next != NULL) {
		replacement->next->prev = replacement;
	}
	if (replacement->prev != NULL) {
		replacement->prev->next = replacement;
	}
	if (parent->child == item) {
		parent->child = replacement;
	}

	item->next = NULL;
	item->prev = NULL;
	cJSON_Delete(item);

	return true;
}

CJSON_PUBLIC(void) cJSON_ReplaceItemInArray(cJSON *array, int which,
		cJSON *newitem) {
	if (which < 0) {
		return;
	}

	cJSON_ReplaceItemViaPointer(array, get_array_item(array, (size_t) which),
			newitem);
}

ICACHE_FLASH_ATTR static cJSON_bool replace_item_in_object(cJSON *object,
		const char *string, cJSON *replacement, cJSON_bool case_sensitive) {
	if ((replacement == NULL) || (string == NULL)) {
		return false;
	}

	/* replace the name in the replacement */
	if (!(replacement->type & cJSON_StringIsConst)
			&& (replacement->string != NULL)) {
		cJSON_free(replacement->string);
	}
	replacement->string = (char*) cJSON_strdup((const unsigned char*) string,
			&global_hooks);
	replacement->type &= ~cJSON_StringIsConst;

	cJSON_ReplaceItemViaPointer(object,
			get_object_item(object, string, case_sensitive), replacement);

	return true;
}

CJSON_PUBLIC(void) cJSON_ReplaceItemInObject(cJSON *object, const char *string,
		cJSON *newitem) {
	replace_item_in_object(object, string, newitem, false);
}

CJSON_PUBLIC(void) cJSON_ReplaceItemInObjectCaseSensitive(cJSON *object,
		const char *string, cJSON *newitem) {
	replace_item_in_object(object, string, newitem, true);
}

/****************************************************************/
// cJSON_CreateXXX function
/* Create basic types: */
CJSON_PUBLIC(cJSON *) cJSON_CreateNull(void)
{
	cJSON *item = cJSON_New_Item(&global_hooks);
	if(item)
	{
		item->type = cJSON_NULL;
	}

	return item;
}

CJSON_PUBLIC(cJSON *) cJSON_CreateTrue(void)
{
	cJSON *item = cJSON_New_Item(&global_hooks);
	if(item)
	{
		item->type = cJSON_True;
	}

	return item;
}

CJSON_PUBLIC(cJSON *) cJSON_CreateFalse(void)
{
	cJSON *item = cJSON_New_Item(&global_hooks);
	if(item)
	{
		item->type = cJSON_False;
	}

	return item;
}

CJSON_PUBLIC(cJSON *) cJSON_CreateBool(cJSON_bool b)
{
	cJSON *item = cJSON_New_Item(&global_hooks);
	if(item)
	{
		item->type = b ? cJSON_True : cJSON_False;
	}

	return item;
}

/*
 * function: cJSON_CreateNumber
 * parameter: double num
 * return: CJSON_PUBLIC(cJSON *)
 * description:
 */
CJSON_PUBLIC(cJSON *) cJSON_CreateNumber(double num)
{
	cJSON *item = cJSON_New_Item(&global_hooks);	// 构建一个新的item
	if(item)	// 如果不为空，则设置数值
	{
		item->type = cJSON_Number;
		item->valuedouble = num;

		/* use saturation in case of overflow */
		if (num >= INT_MAX)
		{
			item->valueint = INT_MAX;
		}
		else if (num <= INT_MIN)
		{
			item->valueint = INT_MIN;
		}
		else
		{
			item->valueint = (int)num;
		}
	}

	return item;
}

/*
 * function: cJSON_CreateString
 * parameter: const char *string
 * return: CJSON_PUBLIC(cJSON *)
 * description:
 */
CJSON_PUBLIC(cJSON *) cJSON_CreateString(const char *string)
{
	cJSON *item = cJSON_New_Item(&global_hooks);
	if(item)
	{
		item->type = cJSON_String;
		item->valuestring = (char*)cJSON_strdup((const unsigned char*)string, &global_hooks);
		if(!item->valuestring)
		{
			cJSON_Delete(item);
			return NULL;
		}
	}

	return item;
}

/*
 * function: cJSON_CreateRaw
 * parameter: const char *raw
 * return: CJSON_PUBLIC(cJSON *)
 * description:
 */
CJSON_PUBLIC(cJSON *) cJSON_CreateRaw(const char *raw)
{
	cJSON *item = cJSON_New_Item(&global_hooks);
	if(item)
	{
		item->type = cJSON_Raw;
		item->valuestring = (char*)cJSON_strdup((const unsigned char*)raw, &global_hooks);
		if(!item->valuestring)
		{
			cJSON_Delete(item);
			return NULL;
		}
	}

	return item;
}

/*
 * function: cJSON_CreateArray
 * return: CJSON_PUBLIC(cJSON *)
 * description: 创建一个Array
 */
CJSON_PUBLIC(cJSON *) cJSON_CreateArray(void)
{
	cJSON *item = cJSON_New_Item(&global_hooks);
	if(item)
	{
		item->type=cJSON_Array;
	}

	return item;
}

/*
 * function: cJSON_CreateObject
 * return: CJSON_PUBLIC(cJSON *)
 * description: 创建一个对象
 */
CJSON_PUBLIC(cJSON *) cJSON_CreateObject(void)
{
	cJSON *item = cJSON_New_Item(&global_hooks);
	if (item)
	{
		item->type = cJSON_Object;
	}

	return item;
}

/* Create Arrays: */

	/*
 * function: cJSON_CreateIntArray
 * parameter: const int *numbers - 一个包含int的数组
 *            int count - 数组长度
 * return: CJSON_PUBLIC(cJSON *)
 * description: 以循环的方式构建一个Int Array
 */
CJSON_PUBLIC(cJSON *) cJSON_CreateIntArray(const int *numbers, int count)
{
	size_t i = 0;
	cJSON *n = NULL;
	cJSON *p = NULL;
	cJSON *a = NULL;

	// 检查参数
	if ((count < 0) || (numbers == NULL))
	{
		return NULL;
	}

	a = cJSON_CreateArray();
	for(i = 0; a && (i < (size_t)count); i++)
	{
		n = cJSON_CreateNumber(numbers[i]);
		if (!n)
		{
			cJSON_Delete(a);
			return NULL;
		}
		if(!i)	// 如果 i == 0
		{
			a->child = n;
		}
		else
		{
			suffix_object(p, n);	// 设置数组链表
		}
		p = n;	// 把当前节点缓存起来，作为上一节点
	}

	return a;
}

	/*
 * function: cJSON_CreateFloatArray
 * return: CJSON_PUBLIC(cJSON *)
 * description: 构建一个Float Array，ESP8266不支持float所以本函数仅仅作为保留
 */
CJSON_PUBLIC(cJSON *) cJSON_CreateFloatArray(const float *numbers, int count)
{
	size_t i = 0;
	cJSON *n = NULL;
	cJSON *p = NULL;
	cJSON *a = NULL;

	if ((count < 0) || (numbers == NULL))
	{
		return NULL;
	}

	a = cJSON_CreateArray();

	for(i = 0; a && (i < (size_t)count); i++)
	{
		n = cJSON_CreateNumber((double)numbers[i]);
		if(!n)
		{
			cJSON_Delete(a);
			return NULL;
		}
		if(!i)
		{
			a->child = n;
		}
		else
		{
			suffix_object(p, n);
		}
		p = n;
	}

	return a;
}

/*
 * function: cJSON_CreateDoubleArray
 * return: CJSON_PUBLIC(cJSON *)
 * description: 构建一个Double Array，ESP8266不支持double所以本函数仅仅作为保留
 */
CJSON_PUBLIC(cJSON *) cJSON_CreateDoubleArray(const double *numbers, int count)
{
	size_t i = 0;
	cJSON *n = NULL;
	cJSON *p = NULL;
	cJSON *a = NULL;

	if ((count < 0) || (numbers == NULL))
	{
		return NULL;
	}

	a = cJSON_CreateArray();

	for(i = 0;a && (i < (size_t)count); i++)
	{
		n = cJSON_CreateNumber(numbers[i]);
		if(!n)
		{
			cJSON_Delete(a);
			return NULL;
		}
		if(!i)
		{
			a->child = n;
		}
		else
		{
			suffix_object(p, n);
		}
		p = n;
	}

	return a;
}

	/*
 * function: cJSON_CreateStringArray
 * return: CJSON_PUBLIC(cJSON *)
 * description: 构建一个字符串Array
 */
CJSON_PUBLIC(cJSON *) cJSON_CreateStringArray(const char **strings, int count)
{
	size_t i = 0;
	cJSON *n = NULL;
	cJSON *p = NULL;
	cJSON *a = NULL;

	if ((count < 0) || (strings == NULL))
	{
		return NULL;
	}

	a = cJSON_CreateArray();

	for (i = 0; a && (i < (size_t)count); i++)
	{
		n = cJSON_CreateString(strings[i]);
		if(!n)
		{
			cJSON_Delete(a);
			return NULL;
		}
		if(!i)
		{
			a->child = n;
		}
		else
		{
			suffix_object(p,n);
		}
		p = n;
	}

	return a;
}

/****************************************************************/

/* Duplication */
CJSON_PUBLIC(cJSON *) cJSON_Duplicate(const cJSON *item, cJSON_bool recurse)
{
	cJSON *newitem = NULL;
	cJSON *child = NULL;
	cJSON *next = NULL;
	cJSON *newchild = NULL;

	/* Bail on bad ptr */
	if (!item)
	{
		goto fail;
	}
	/* Create new item */
	newitem = cJSON_New_Item(&global_hooks);
	if (!newitem)
	{
		goto fail;
	}
	/* Copy over all vars */
	newitem->type = item->type & (~cJSON_IsReference);
	newitem->valueint = item->valueint;
	newitem->valuedouble = item->valuedouble;
	if (item->valuestring)
	{
		newitem->valuestring = (char*)cJSON_strdup((unsigned char*)item->valuestring, &global_hooks);
		if (!newitem->valuestring)
		{
			goto fail;
		}
	}
	if (item->string)
	{
		newitem->string = (item->type&cJSON_StringIsConst) ? item->string : (char*)cJSON_strdup((unsigned char*)item->string, &global_hooks);
		if (!newitem->string)
		{
			goto fail;
		}
	}
	/* If non-recursive, then we're done! */
	if (!recurse)
	{
		return newitem;
	}
	/* Walk the ->next chain for the child. */
	child = item->child;
	while (child != NULL)
	{
		newchild = cJSON_Duplicate(child, true); /* Duplicate (with recurse) each item in the ->next chain */
		if (!newchild)
		{
			goto fail;
		}
		if (next != NULL)
		{
			/* If newitem->child already set, then crosswire ->prev and ->next and move on */
			next->next = newchild;
			newchild->prev = next;
			next = newchild;
		}
		else
		{
			/* Set newitem->child and move to it */
			newitem->child = newchild;
			next = newchild;
		}
		child = child->next;
	}

	return newitem;

	fail:
	if (newitem != NULL)
	{
		cJSON_Delete(newitem);
	}

	return NULL;
}

	/*
 * function: cJSON_Minify
 * description: 去除字符串中的空格、\t、\r、\n等字符
 */
CJSON_PUBLIC(void) cJSON_Minify(char *json) {
	unsigned char *into = (unsigned char*) json;

	if (json == NULL) {
		return;
	}

	while (*json) {
		if (*json == ' ') {
			json++;
		} else if (*json == '\t') {
			/* Whitespace characters. */
			json++;
		} else if (*json == '\r') {
			json++;
		} else if (*json == '\n') {
			json++;
		} else if ((*json == '/') && (json[1] == '/')) {
			/* double-slash comments, to end of line. */
			while (*json && (*json != '\n')) {
				json++;
			}
		} else if ((*json == '/') && (json[1] == '*')) {
			/* multiline comments. */
			while (*json && !((*json == '*') && (json[1] == '/'))) {
				json++;
			}
			json += 2;
		} else if (*json == '\"') {
			/* string literals, which are \" sensitive. */
			*into++ = (unsigned char) *json++;
			while (*json && (*json != '\"')) {
				if (*json == '\\') {
					*into++ = (unsigned char) *json++;
				}
				*into++ = (unsigned char) *json++;
			}
			*into++ = (unsigned char) *json++;
		} else {
			/* All other characters. */
			*into++ = (unsigned char) *json++;
		}
	}

	/* and null-terminate. */
	*into = '\0';
}

/****************************************************************/
// isXXX Function
// 判断item的类型用
CJSON_PUBLIC(cJSON_bool) cJSON_IsInvalid(const cJSON * const item) {
	if (item == NULL) {
		return false;
	}

	return (item->type & 0xFF) == cJSON_Invalid;
}

CJSON_PUBLIC(cJSON_bool) cJSON_IsFalse(const cJSON * const item) {
	if (item == NULL) {
		return false;
	}

	return (item->type & 0xFF) == cJSON_False;
}

CJSON_PUBLIC(cJSON_bool) cJSON_IsTrue(const cJSON * const item) {
	if (item == NULL) {
		return false;
	}

	return (item->type & 0xff) == cJSON_True;
}

CJSON_PUBLIC(cJSON_bool) cJSON_IsBool(const cJSON * const item) {
	if (item == NULL) {
		return false;
	}

	return (item->type & (cJSON_True | cJSON_False)) != 0;
}
CJSON_PUBLIC(cJSON_bool) cJSON_IsNull(const cJSON * const item) {
	if (item == NULL) {
		return false;
	}

	return (item->type & 0xFF) == cJSON_NULL;
}

CJSON_PUBLIC(cJSON_bool) cJSON_IsNumber(const cJSON * const item) {
	if (item == NULL) {
		return false;
	}

	return (item->type & 0xFF) == cJSON_Number;
}

CJSON_PUBLIC(cJSON_bool) cJSON_IsString(const cJSON * const item) {
	if (item == NULL) {
		return false;
	}

	return (item->type & 0xFF) == cJSON_String;
}

CJSON_PUBLIC(cJSON_bool) cJSON_IsArray(const cJSON * const item) {
	if (item == NULL) {
		return false;
	}

	return (item->type & 0xFF) == cJSON_Array;
}

CJSON_PUBLIC(cJSON_bool) cJSON_IsObject(const cJSON * const item) {
	if (item == NULL) {
		return false;
	}

	return (item->type & 0xFF) == cJSON_Object;
}

CJSON_PUBLIC(cJSON_bool) cJSON_IsRaw(const cJSON * const item) {
	if (item == NULL) {
		return false;
	}

	return (item->type & 0xFF) == cJSON_Raw;
}

/****************************************************************/

CJSON_PUBLIC(cJSON_bool) cJSON_Compare(const cJSON * const a,
		const cJSON * const b, const cJSON_bool case_sensitive) {
	if ((a == NULL) || (b == NULL) || ((a->type & 0xFF) != (b->type & 0xFF))
			|| cJSON_IsInvalid(a)) {
		return false;
	}

	/* check if type is valid */
	switch (a->type & 0xFF) {
	case cJSON_False:
	case cJSON_True:
	case cJSON_NULL:
	case cJSON_Number:
	case cJSON_String:
	case cJSON_Raw:
	case cJSON_Array:
	case cJSON_Object:
		break;

	default:
		return false;
	}

	/* identical objects are equal */
	if (a == b) {
		return true;
	}

	switch (a->type & 0xFF) {
	/* in these cases and equal type is enough */
	case cJSON_False:
	case cJSON_True:
	case cJSON_NULL:
		return true;

	case cJSON_Number:
		if (a->valuedouble == b->valuedouble) {
			return true;
		}
		return false;

	case cJSON_String:
	case cJSON_Raw:
		if ((a->valuestring == NULL) || (b->valuestring == NULL)) {
			return false;
		}
		if (strcmp(a->valuestring, b->valuestring) == 0) {
			return true;
		}

		return false;

	case cJSON_Array: {
		cJSON *a_element = a->child;
		cJSON *b_element = b->child;

		for (; (a_element != NULL) && (b_element != NULL);) {
			if (!cJSON_Compare(a_element, b_element, case_sensitive)) {
				return false;
			}

			a_element = a_element->next;
			b_element = b_element->next;
		}

		/* one of the arrays is longer than the other */
		if (a_element != b_element) {
			return false;
		}

		return true;
	}

	case cJSON_Object: {
		cJSON *a_element = NULL;
		cJSON *b_element = NULL;
		cJSON_ArrayForEach(a_element, a)
		{
			/* TODO This has O(n^2) runtime, which is horrible! */
			b_element = get_object_item(b, a_element->string, case_sensitive);
			if (b_element == NULL) {
				return false;
			}

			if (!cJSON_Compare(a_element, b_element, case_sensitive)) {
				return false;
			}
		}

		/* doing this twice, once on a and b to prevent true comparison if a subset of b
		 * TODO: Do this the proper way, this is just a fix for now */
		cJSON_ArrayForEach(b_element, b)
		{
			a_element = get_object_item(a, b_element->string, case_sensitive);
			if (a_element == NULL) {
				return false;
			}

			if (!cJSON_Compare(b_element, a_element, case_sensitive)) {
				return false;
			}
		}

		return true;
	}

	default:
		return false;
	}
}

/****************************************************************/

CJSON_PUBLIC(void *) cJSON_malloc(size_t size)
{
	return global_hooks.allocate(size);
}

CJSON_PUBLIC(void) cJSON_free(void *object) {
	global_hooks.deallocate(object);
}
