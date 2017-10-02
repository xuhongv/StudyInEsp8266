/**
 * Project page: https://github.com/wangrn/ringbuffer
 * Copyright (c) 2013 Wang Ruining <https://github.com/wangrn>
 * @date 2013/01/16 13:33:20
 * @brief   a simple ringbuffer, DO NOT support dynamic expanded memory
 */
#ifndef _GIZWITS_RING_BUFFER_H
#define _GIZWITS_RING_BUFFER_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define min(a, b) (a)<(b)?(a):(b)                   ///< Calculate the minimum value

typedef struct {
    size_t rbCapacity;
    uint8_t  *rbHead;
    uint8_t  *rbTail;
    uint8_t  *rbBuff;
}rb_t;

int8_t rbCreate(rb_t* rb);
int8_t rbDelete(rb_t* rb);
int32_t rbCapacity(rb_t *rb);
int32_t rbCanRead(rb_t *rb);
int32_t rbCanWrite(rb_t *rb);
int32_t rbRead(rb_t *rb, void *data, size_t count);
int32_t rbWrite(rb_t *rb, const void *data, size_t count);

#endif
