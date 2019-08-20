//
//  objc-os.h
//  ZQJITEngine
//
//  Created by zuqingxie on 2019/4/9.
//  Copyright © 2019年 zqx. All rights reserved.
//

#ifndef objc_os_h
#define objc_os_h

#include "objc-config.h"

#ifdef __LP64__
#   define WORD_SHIFT 3UL
#   define WORD_MASK 7UL
#   define WORD_BITS 64
#else
#   define WORD_SHIFT 2UL
#   define WORD_MASK 3UL
#   define WORD_BITS 32
#endif

static inline void *
memdup(const void *mem, size_t len)
{
    void *dup = malloc(len);
    memcpy(dup, mem, len);
    return dup;
}

#if (TARGET_OS_OSX || TARGET_OS_IOSMAC) && __x86_64__
// 64-bit Mac - tag bit is LSB
#   define OBJC_MSB_TAGGED_POINTERS 0
#else
// Everything else - tag bit is MSB
#   define OBJC_MSB_TAGGED_POINTERS 1
#endif

#define _OBJC_TAG_INDEX_MASK 0x7
// array slot includes the tag bit itself
#define _OBJC_TAG_SLOT_COUNT 16
#define _OBJC_TAG_SLOT_MASK 0xf

#define _OBJC_TAG_EXT_INDEX_MASK 0xff
// array slot has no extra bits
#define _OBJC_TAG_EXT_SLOT_COUNT 256
#define _OBJC_TAG_EXT_SLOT_MASK 0xff

#if OBJC_MSB_TAGGED_POINTERS
#   define _OBJC_TAG_MASK (1UL<<63)
#   define _OBJC_TAG_INDEX_SHIFT 60
#   define _OBJC_TAG_SLOT_SHIFT 60
#   define _OBJC_TAG_PAYLOAD_LSHIFT 4
#   define _OBJC_TAG_PAYLOAD_RSHIFT 4
#   define _OBJC_TAG_EXT_MASK (0xfUL<<60)
#   define _OBJC_TAG_EXT_INDEX_SHIFT 52
#   define _OBJC_TAG_EXT_SLOT_SHIFT 52
#   define _OBJC_TAG_EXT_PAYLOAD_LSHIFT 12
#   define _OBJC_TAG_EXT_PAYLOAD_RSHIFT 12
#else
#   define _OBJC_TAG_MASK 1UL
#   define _OBJC_TAG_INDEX_SHIFT 1
#   define _OBJC_TAG_SLOT_SHIFT 0
#   define _OBJC_TAG_PAYLOAD_LSHIFT 0
#   define _OBJC_TAG_PAYLOAD_RSHIFT 4
#   define _OBJC_TAG_EXT_MASK 0xfUL
#   define _OBJC_TAG_EXT_INDEX_SHIFT 4
#   define _OBJC_TAG_EXT_SLOT_SHIFT 4
#   define _OBJC_TAG_EXT_PAYLOAD_LSHIFT 0
#   define _OBJC_TAG_EXT_PAYLOAD_RSHIFT 12
#endif

#if SUPPORT_TAGGED_POINTERS

extern "C" {
    extern Class objc_debug_taggedpointer_classes[_OBJC_TAG_SLOT_COUNT*2];
    extern Class objc_debug_taggedpointer_ext_classes[_OBJC_TAG_EXT_SLOT_COUNT];
}
#define objc_tag_classes objc_debug_taggedpointer_classes
#define objc_tag_ext_classes objc_debug_taggedpointer_ext_classes

#endif

static inline bool
_objc_isTaggedPointer(const void * _Nullable ptr)
{
    return ((uintptr_t)ptr & _OBJC_TAG_MASK) == _OBJC_TAG_MASK;
}

static uintptr_t objc_debug_taggedpointer_obfuscator = 0;

static inline uintptr_t
_objc_decodeTaggedPointer(const void * _Nullable ptr)
{
    return (uintptr_t)ptr ^ objc_debug_taggedpointer_obfuscator;
}

#if __LP64__
typedef uint32_t mask_t;  // x86_64 & arm64 asm are less efficient with 16-bits
#else
typedef uint16_t mask_t;
#endif

// Leaks-compatible version that steals low bits only.

// class or superclass has .cxx_construct implementation
#define RW_HAS_CXX_CTOR       (1<<18)
// class or superclass has .cxx_destruct implementation
#define RW_HAS_CXX_DTOR       (1<<17)
// class or superclass has default alloc/allocWithZone: implementation
// Note this is is stored in the metaclass.
#define RW_HAS_DEFAULT_AWZ    (1<<16)
// class's instances requires raw isa
#define RW_REQUIRES_RAW_ISA   (1<<15)

// class is a Swift class from the pre-stable Swift ABI
#define FAST_IS_SWIFT_LEGACY    (1UL<<0)
// class is a Swift class from the stable Swift ABI
#define FAST_IS_SWIFT_STABLE    (1UL<<1)
// class or superclass has default retain/release/autorelease/retainCount/
//   _tryRetain/_isDeallocating/retainWeakReference/allowsWeakReference
#define FAST_HAS_DEFAULT_RR     (1UL<<2)
// data pointer
#define FAST_DATA_MASK          0x00007ffffffffff8UL


#define RO_META               (1<<0)
// class is a root class
#define RO_ROOT               (1<<1)
// class has .cxx_construct/destruct implementations
#define RO_HAS_CXX_STRUCTORS  (1<<2)
// class has +load implementation
// #define RO_HAS_LOAD_METHOD    (1<<3)
// class has visibility=hidden set
#define RO_HIDDEN             (1<<4)
// class has attribute(objc_exception): OBJC_EHTYPE_$_ThisClass is non-weak
#define RO_EXCEPTION          (1<<5)
// this bit is available for reassignment
// #define RO_REUSE_ME           (1<<6)
// class compiled with ARC
#define RO_IS_ARC             (1<<7)
// class has .cxx_destruct but no .cxx_construct (with RO_HAS_CXX_STRUCTORS)
#define RO_HAS_CXX_DTOR_ONLY  (1<<8)
// class is not ARC but has ARC-style weak ivar layout
#define RO_HAS_WEAK_WITHOUT_ARC (1<<9)

// class is in an unloadable bundle - must never be set by compiler
#define RO_FROM_BUNDLE        (1<<29)
// class is unrealized future class - must never be set by compiler
#define RO_FUTURE             (1<<30)
// class is realized - must never be set by compiler
#define RO_REALIZED           (1<<31)

// Values for class_rw_t->flags
// These are not emitted by the compiler and are never used in class_ro_t.
// Their presence should be considered in future ABI versions.
// class_t->data is class_rw_t, not class_ro_t
#define RW_REALIZED           (1<<31)
// class is unresolved future class
#define RW_FUTURE             (1<<30)
// class is initialized
#define RW_INITIALIZED        (1<<29)
// class is initializing
#define RW_INITIALIZING       (1<<28)
// class_rw_t->ro is heap copy of class_ro_t
#define RW_COPIED_RO          (1<<27)
// class allocated but not yet registered
#define RW_CONSTRUCTING       (1<<26)
// class allocated and registered
#define RW_CONSTRUCTED        (1<<25)
// available for use; was RW_FINALIZE_ON_MAIN_THREAD
// #define RW_24 (1<<24)
// class +load has been called
#define RW_LOADED             (1<<23)
#if !SUPPORT_NONPOINTER_ISA
// class instances may have associative references
#define RW_INSTANCES_HAVE_ASSOCIATED_OBJECTS (1<<22)
#endif
// class has instance-specific GC layout
#define RW_HAS_INSTANCE_SPECIFIC_LAYOUT (1 << 21)
// available for use
// #define RW_20       (1<<20)
// class has started realizing but not yet completed it
#define RW_REALIZING          (1<<19)

#endif /* objc_os_h */



/*
 * @APPLE_LICENSE_HEADER_START@
 *
 * Copyright (c) 2018 Apple Inc.  All Rights Reserved.
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */
/********************************************************************
 *
 *  isa.h - Definitions of isa fields for C and assembly code.
 *
 ********************************************************************/

#ifndef _OBJC_ISA_H_
#define _OBJC_ISA_H_

//#include "objc-config.h"


#if (!SUPPORT_NONPOINTER_ISA && !SUPPORT_PACKED_ISA && !SUPPORT_INDEXED_ISA) ||\
( SUPPORT_NONPOINTER_ISA &&  SUPPORT_PACKED_ISA && !SUPPORT_INDEXED_ISA) ||\
( SUPPORT_NONPOINTER_ISA && !SUPPORT_PACKED_ISA &&  SUPPORT_INDEXED_ISA)
// good config
#else
#   error bad config
#endif


#if SUPPORT_PACKED_ISA

// extra_rc must be the MSB-most field (so it matches carry/overflow flags)
// nonpointer must be the LSB (fixme or get rid of it)
// shiftcls must occupy the same bits that a real class pointer would
// bits + RC_ONE is equivalent to extra_rc + 1
// RC_HALF is the high bit of extra_rc (i.e. half of its range)

// future expansion:
// uintptr_t fast_rr : 1;     // no r/r overrides
// uintptr_t lock : 2;        // lock for atomic property, @synch
// uintptr_t extraBytes : 1;  // allocated with extra bytes

# if __arm64__
#   define ISA_MASK        0x0000000ffffffff8ULL
#   define ISA_MAGIC_MASK  0x000003f000000001ULL
#   define ISA_MAGIC_VALUE 0x000001a000000001ULL
#   define ISA_BITFIELD                                                      \
uintptr_t nonpointer        : 1;                                       \
uintptr_t has_assoc         : 1;                                       \
uintptr_t has_cxx_dtor      : 1;                                       \
uintptr_t shiftcls          : 33; /*MACH_VM_MAX_ADDRESS 0x1000000000*/ \
uintptr_t magic             : 6;                                       \
uintptr_t weakly_referenced : 1;                                       \
uintptr_t deallocating      : 1;                                       \
uintptr_t has_sidetable_rc  : 1;                                       \
uintptr_t extra_rc          : 19
#   define RC_ONE   (1ULL<<45)
#   define RC_HALF  (1ULL<<18)

# elif __x86_64__
#   define ISA_MASK        0x00007ffffffffff8ULL
#   define ISA_MAGIC_MASK  0x001f800000000001ULL
#   define ISA_MAGIC_VALUE 0x001d800000000001ULL
#   define ISA_BITFIELD                                                        \
uintptr_t nonpointer        : 1;                                         \
uintptr_t has_assoc         : 1;                                         \
uintptr_t has_cxx_dtor      : 1;                                         \
uintptr_t shiftcls          : 44; /*MACH_VM_MAX_ADDRESS 0x7fffffe00000*/ \
uintptr_t magic             : 6;                                         \
uintptr_t weakly_referenced : 1;                                         \
uintptr_t deallocating      : 1;                                         \
uintptr_t has_sidetable_rc  : 1;                                         \
uintptr_t extra_rc          : 8
#   define RC_ONE   (1ULL<<56)
#   define RC_HALF  (1ULL<<7)

# else
#   error unknown architecture for packed isa
# endif

// SUPPORT_PACKED_ISA
#endif


#if SUPPORT_INDEXED_ISA

# if  __ARM_ARCH_7K__ >= 2  ||  (__arm64__ && !__LP64__)
// armv7k or arm64_32

#   define ISA_INDEX_IS_NPI_BIT  0
#   define ISA_INDEX_IS_NPI_MASK 0x00000001
#   define ISA_INDEX_MASK        0x0001FFFC
#   define ISA_INDEX_SHIFT       2
#   define ISA_INDEX_BITS        15
#   define ISA_INDEX_COUNT       (1 << ISA_INDEX_BITS)
#   define ISA_INDEX_MAGIC_MASK  0x001E0001
#   define ISA_INDEX_MAGIC_VALUE 0x001C0001
#   define ISA_BITFIELD                         \
uintptr_t nonpointer        : 1;          \
uintptr_t has_assoc         : 1;          \
uintptr_t indexcls          : 15;         \
uintptr_t magic             : 4;          \
uintptr_t has_cxx_dtor      : 1;          \
uintptr_t weakly_referenced : 1;          \
uintptr_t deallocating      : 1;          \
uintptr_t has_sidetable_rc  : 1;          \
uintptr_t extra_rc          : 7
#   define RC_ONE   (1ULL<<25)
#   define RC_HALF  (1ULL<<6)

# else
#   error unknown architecture for indexed isa
# endif

// SUPPORT_INDEXED_ISA
#endif


// _OBJC_ISA_H_
#endif

