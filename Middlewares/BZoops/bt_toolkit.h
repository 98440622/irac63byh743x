/**
 * @file bt_toolkit.h
 * @author 98440622 (98440622@163.com)
 * @brief TBD
 * @version 1.0
 * @date 2020-02-24 Create
 * @date 2020--4-28 Add compiler version configuration for BT_TRACE
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#ifndef BT_TOOLKIT_H_
#define BT_TOOLKIT_H_

#if defined(CC_VER_99)
    #include <stdio.h>
    #define BT_TRACE(fmt, ...) printf("[%s%s@%d] "##fmt, __FILE__, __FUNCTION__, __LINE__,##__VA_ARGS__)
#elif defined(CC_VER_98)
    #include <stdio.h>
    #define BT_TRACE(fmt, ...) printf(fmt)
#else 
   #error "CC_VER not defined"
#endif

// ////////////////////////////////////////////////////////////////////////////
//
//
#define BT_SET_BIT(type, name, bit) \
   (name |= ((type)(1) << bit))
#define BT_CLR_BIT(type, name, bit) \
   (name &= ~((type)(1) << bit))

// ////////////////////////////////////////////////////////////////////////////
//
//
#define BT_COUNTER(type, name) \
   type name

#define BT_CLEAR_COUNTER(name) \
   (name = 0)

#define BT_SET_COUNTER(name, val) \
   (name = val)

#define BT_IS_COUNTER_END(name) \
   ((name == 0) ? 1 : 0)

#define BT_DO_COUNTER(name) \
   ((name > 0) ? name-- : 0)

#define BT_COUNTER_CONDITION(name, cond, val) \
   ((cond) ? ((name == 0) ? 1 : 0) : ((name = val), 0))


// ////////////////////////////////////////////////////////////////////////////
//
//
#define BT_OFFSETOF(S, m) \
   (size_t)&(((S*)0)->m)

#define BT_OFFSETOFSIZE(S, m) \
   sizeof(((S*)0)->m)


// ////////////////////////////////////////////////////////////////////////////
//
//
/*!
   BT_CIRCULAR_QUEUE

   Example:
   <code>
   BT_CIRCULAR_QUEUE(int, dummy, 8)[2];

   void foo()
   {
      int i = -8;
      BT_INI_CIRCULAR_QUEUE(dummy[0], 8);
      while (!BT_IS_CIRCULAR_QUEUE_FULL(dummy[0])) {
         BT_CIRCULAR_QUEUE_PUSH(dummy[0], i++);
      }

      while (!BT_IS_CIRCULAR_QUEUE_EMPTY(dummy[0])) {
         BT_CIRCULAR_QUEUE_POP(dummy[0], &i);
         BT_BTRACE("%d ", i);
      }
   }
   </code>
 */
// This macro must be set before use CIRCULAR_QUEUE 'functions'.
// The max size is 255!

#define BT_CIRCULAR_QUEUE(type, name, length) struct _tag##name { \
   type container[length + 1]; \
   unsigned char head; \
   unsigned char tail; \
   unsigned char size; \
} name

#define BT_INI_CIRCULAR_QUEUE(name, length) ( \
   name.head = 0, \
   name.tail = 0, \
   name.size = length + 1 \
)

#define BT_IS_CIRCULAR_QUEUE_EMPTY(name) \
   (name.tail == name.head ? 1 : 0)

#define BT_IS_CIRCULAR_QUEUE_FULL(name) \
   (((name.head + 1) % name.size == name.tail) ? 1 : 0)

#define BT_CIRCULAR_QUEUE_PUSH(name, item) ( \
   name.container[name.head] = item, \
   name.head = (name.head + 1) % name.size, \
   name.tail = (name.head == name.tail) ? ((name.tail + 1) % name.size) : name.tail\
)

#define BT_CIRCULAR_QUEUE_POP(name, pitem) ( \
   *(pitem) = name.container[name.tail], \
   name.tail = (name.tail + 1) % name.size \
)

#define BT_CIRCULAR_QUEUE_SIZE(name) ( \
   (name.head >= name.tail) ? (name.head - name.tail) \
   : (name.head + name.size - name.tail) \
)

#endif // BT_TOOLKIT_H_

