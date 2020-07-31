/**
 * @file bzObjects.h
 * @author ZHANG BoTong
 * @brief Each type of object should include one instance of this class.
 * 
 *        There are three files to define an object obj.h obj.inc and obj.c.
 * 
 *          - obj.h file contains the properties, methods and interfaces of 
 *            an object. 
 *          - obj.inc file contains the implementation of the methods and the
 *          - obj.c file is generally treated as implementation of interfaces
 *            which is also should be visibly included into project (makefile)
 *            file.
 * 
 *        It is the client's duty to give the implementation to the interfaces
 *        which are also kinds of bridge to the client code.
 * 
 * @version 0.1
 * @date 2020-04-02 Created
 * @date 2020-04-17 Add brief to programming pattern applying this OO framework
 * @date 2020-05-12 Add object configuration control
 * 
 * @copyright Free Copyright (c) 2020
 * 
 */
#ifndef _BZOBJECTS_H_
#define _BZOBJECTS_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "bt_typedefs.h"
#include "bt_toolkit.h"

#undef _OVERRIDE_
#define _OVERRIDE_
#undef _override_
#define _override_

#undef _PUBLIC_
#define _PUBLIC_
#undef _public_
#define _public_

#undef _PRIVATE_
#define _PRIVATE_
#undef _private_
#define _private_

#undef _INTERFACE_
#define _INTERFACE_
#undef _interface_
#define _interface_

#undef _IN_
#define _IN_
#undef _in_
#define _in_

#undef _OUT_
#define _OUT_
#undef _out_
#define _out_

#define bzOBJ(name)   bz##name

#define _NONE_OS    0
#define _WIN32_OS	1
#define _UCOSIII_OS 2
#define _VXWORKS_OS 3

#if BZOBJ_OS == _WIN32_OS
    #undef RESOURCE_LOCK
    #define RESOURCE_LOCK()

    #undef RESOURCE_UNLOCK
    #define RESOURCE_UNLOCK()

    #undef bzLOG
    #define bzLOG printf
    #undef bzDBG
    #define bzDBG BT_TRACE
#elif BZOBJ_OS == _UCOSIII_OS
    #undef RESOURCE_LOCK
    #define RESOURCE_LOCK()

    #undef RESOURCE_UNLOCK
    #define RESOURCE_UNLOCK()

    #undef bzLOG
    #define bzLOG OS_TRACE
    #undef bzDBG
    #define bzDBG BT_TRACE
#elif BZOBJ_OS == _VXWORKS_OS
    #undef RESOURCE_LOCK
    #define RESOURCE_LOCK()

    #undef RESOURCE_UNLOCK
    #define RESOURCE_UNLOCK()
    #undef bzLOG
    #define bzLOG printf
    #undef bzDBG
    #define bzDBG BT_TRACE
#elif BZOBJ_OS == _NONE_OS
    #undef RESOURCE_LOCK
    #define RESOURCE_LOCK()

    #undef RESOURCE_UNLOCK
    #define RESOURCE_UNLOCK()
    #undef bzLOG
    #define bzLOG printf
    #undef bzDBG
    #define bzDBG BT_TRACE
#endif // BZOBJ_OS

typedef unsigned char   bzByte;
typedef char            bzCHAR;
typedef char*           bzLPCHAR;
typedef const char*     bzCLPCHAR;

typedef bt_int16        bzRESULT;

typedef bt_uint8        bzU8;
typedef bt_int8         bzI8;
typedef bt_uint16       bzU16;
typedef bt_int16        bzI16;
typedef bt_uint32       bzU32;
typedef bt_int32        bzI32;
typedef bt_uint64       bzU64;
typedef bt_int64        bzI64;
typedef bt_real32       bzR32;
typedef bt_real64       bzR64;

typedef bt_boolean      bzBool;
#undef bzTrue           
#define bzTrue          (1)
#undef bzFalse
#define bzFalse         (0)

#undef bzNull           
#define bzNull          ((void*)0)

/**
 * @brief This is the sample to show how an object should be defined
 * 
 * 
 * @code
 *   typedef struct _tagbzObject {
 *       _PRIVATE_ bt_boolean (*doMyOwnTask) (struct _tagbzObject*);
 *       
 *       _OVERRIDE_ _PUBLIC_ void (*updateMyself) (struct _tagbzObject*, _IN_ int*, _OUT_ int*);
 *   } bzObject;
 *
 *   _OVERRIDE_ void bzObject_ctor(bzObject*);
 *   _OVERRIDE_ void bzObject_dtor(bzObject*);
 * @code
 * 
 */

/****************************************************************************
 *                O B J E C T     C O N F I G E R A T I O N S    
 ***************************************************************************/ 

#define bzApplication_INCLUDED              (1)
#define bzBusAdapter_INCLUDED               (1)
#define bzBusInterface_INCLUDED             (1)
#define bzCompressor_INCLUDED               (1)
#define bzCondensatePumpkit_INCLUDED        (1)
#define bzDanfossVFD_INCLUDED               (1)
#define bzDAQ_INCLUDED                      (1)
#define bzEEV_INCLUDED                      (1)
#define bzElectricHeater_INCLUDED           (1)
#define bzFan_INCLUDED                      (1)
#define bzNTCSensor_INCLUDED                (1)
#define bzPID_INCLUDED                      (0)
#define bzRelay_INCLUDED                    (1)
#define bzSequenceWatcher_INCLUDED          (1)
#define bzSwitchInput_INCLUDED              (1)
#define bzSimpleTimer_INCLUDED              (0)
#define bzMBNode_INCLUDED                   (1)
#define bzMBDevice_INCLUDED                 (1)
#define bzTimer_INCLUDED                    (1)

/**
 * @brief Define the max number of 1s timer that a bzSimpleTimer can provide
 *
 * Due to the limitation of ID type, the true maximum number is 65536.
 */
#define BZSIMPLETIMER_MAX_1S_TIMERS         (64) // times by 8

#define BZACTIVETIMER_MAX_CAPACITY          (32)

// Max number of devices can be supported on one RS485 port
#define bzMODBUSMASTER_MAX_NBR_OF_DEVICES   (8)
#define bzMODBUSDEVICE_REG_QUEUE_DEPTH      (8)

#ifdef __cplusplus
}
#endif

#endif // _BZOBJECTS_H_
