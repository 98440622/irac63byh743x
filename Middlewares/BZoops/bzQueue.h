/**
 * @file bzQueue.h
 * @author ZHANG BoTong
 * @brief The Message queue is very helpful for multi-task programming.
 * 
 * @version 0.1
 * @date 2020-04-24 Created
 * @date 2020-07-31 Add some samples for bzSimpleQueue
 * 
 * @copyright Free Copyright (c) 2020
 * 
 */
#ifndef _BZQUEUE_H_
#define _BZQUEUE_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <memory.h>
#include "bzObject.h"

/**
 * @brief This is samples of how to use bzSimpleQueue
 * 
 * 
 * @code
 * int mm[4] = {0, 1, 2, 3};
 * int i = 0;
 * 
 * bzSimpleQueue_new(int, testMM, 4);
 * while (!bzSimpleQueue_isFull(testMM))
 *     bzSimpleQueue_push(int, testMM, mm[i++]);
 * while (!bzSimpleQueue_isEmpty(testMM)) {
 *     int m;
 *     bzSimpleQueue_pop(int, testMM, m);
 *     printf("%d ", m);
 * }
 * @endcode
 * 
 * 
 * @code
 * struct _tRect {
 *     int x;
 *     int y;
 * } rc[4] = {
 *     {0, 0},
 *     {0, 1},
 *     {1, 0},
 *     {1, 1},
 * };
 * int i = 0;
 * 
 * bzSimpleQueue_new(struct _tRect, testRC, 4);
 * while (!bzSimpleQueue_isFull(testRC))
 *     bzSimpleQueue_push(struct _tRect, testRC, rc[i++]);
 * while (!bzSimpleQueue_isEmpty(testRC)) {
 *     struct _tRect rt;
 *     bzSimpleQueue_pop(struct _tRect, testRC, rt);
 *     printf("[%d %d]\r\n", rt.x, rt.y);
 * }
 * @endcode
 * 
 */

#define bzSimpleQueue_new(type, name, capacity) \
    struct _tagbzSimpleQueue##name { \
        type buffer[capacity + 1];\
        bt_uint16 head;\
        bt_uint16 tail;\
        bt_uint16 size;\
    } name = {\
        {0},\
        0,\
        0,\
        capacity + 1,\
    };

#define bzSimpleQueue(type, name, capacity) \
    struct _tagbzSimpleQueue##name { \
        type buffer[capacity + 1];\
        bt_uint16 head;\
        bt_uint16 tail;\
        bt_uint16 size;\
    } name

#define bzSimpleQueue_ctor(name, capacity)( \
   name.head = 0, \
   name.tail = 0, \
   name.size = capacity + 1 \
)

#define bzSimpleQueue_isEmpty(name) \
   (name.tail == name.head ? 1 : 0)

#define bzSimpleQueue_isFull(name) \
   (((name.head + 1) % name.size == name.tail) ? 1 : 0)

#define bzSimpleQueue_push(type, name, variable) ( \
   memcpy((void*)&(name.buffer[name.head]), (const void*)&variable, sizeof(type)), \
   name.head = (name.head + 1) % name.size, \
   name.tail = (name.head == name.tail) ? ((name.tail + 1) % name.size) : name.tail\
)

#define bzSimpleQueue_pop(type, name, variable) ( \
   memcpy((void*)&variable, (const void*)&(name.buffer[name.tail]), sizeof(type)), \
   name.tail = (name.tail + 1) % name.size \
)

#define bzSimpleQueue_count(name) ( \
   (name.head >= name.tail) ? (name.head - name.tail) \
   : (name.head + name.size - name.tail) \
)


#ifdef __cplusplus
}
#endif

#endif // _BZQUEUE_H_