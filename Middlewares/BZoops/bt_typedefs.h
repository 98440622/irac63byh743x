/**
 * @file bt_typedefs.h
 * @author 98440622 (98440622@163.com)
 * @brief TBD
 * @version 1.0
 * @date 2020-02-24
 * 
 * @copyright Copyright (c) 2020
 * 
 * @bug 2020-02-24 First create
 * 
 */

#ifndef BT_TYPEDEFS_H_
#define BT_TYPEDEFS_H_

#ifdef CPU_16
   typedef unsigned short  bt_uint16;
   typedef short           bt_int16;
   typedef unsigned int    bt_uint32;
   typedef int             bt_int32;
   typedef unsigned long   bt_uint64;
   typedef long            bt_int64;
#else
   typedef unsigned short  bt_uint16;
   typedef short           bt_int16;
   typedef unsigned int    bt_uint32;
   typedef int             bt_int32;
   typedef unsigned long   bt_uint64;
   typedef long            bt_int64;
#endif

typedef unsigned char   bt_uint8;
typedef char            bt_int8;
typedef float           bt_real32;
typedef double          bt_real64;
typedef bt_uint8        bt_byte;

typedef bt_uint16 bt_boolean;
#define bt_false  0
#define bt_true   1

#define bt_null ((void*)0)

#endif /* BT_TYPEDEFS_H_ */
