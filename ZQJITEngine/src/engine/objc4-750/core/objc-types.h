//
//  objc-types.h
//  ZQJITEngine
//
//  Created by zuqingxie on 2019/4/8.
//  Copyright © 2019年 zqx. All rights reserved.
//

#ifndef objc_types_h
#define objc_types_h

#include <objc/objc.h>
#include <string>
#include "objc-define.h"
#include "objc-utils.h"

namespace zq { namespace objc {
    
    //------------------------------method64_t---------------------------------
    using MethodListIMP = IMP;
    struct method64_t {
        SEL name;
        const char *types;
        MethodListIMP imp;
        
        struct SortBySELAddress :
        public std::binary_function<const method64_t&,
        const method64_t&, bool>
        {
            bool operator() (const method64_t& lhs,
                             const method64_t& rhs)
            { return lhs.name < rhs.name; }
        };
        
        std::string getDebugDescription(int level = 0) const;
    };
    //------------------------------method64_t---------------------------------
    
    
    //------------------------------method64_list_t---------------------------------
    struct method64_list_t : entsize64_list_tt<method64_t, method64_list_t, 0x3> {
        bool isFixedUp() const;
        void setFixedUp();
        
        uint32_t indexOfMethod(const method64_t *meth) const {
            uint32_t i =
            (uint32_t)(((uintptr_t)meth - (uintptr_t)this) / entsize());
            assert(i < count);
            return i;
        }
        
        std::string getDebugDescription(int level) const;
    };
    //------------------------------method64_list_t---------------------------------
    
    
    //------------------------------ivar64_t---------------------------------
    struct ivar64_t {
#if __x86_64__
        // *offset was originally 64-bit on some x86_64 platforms.
        // We read and write only 32 bits of it.
        // Some metadata provides all 64 bits. This is harmless for unsigned
        // little-endian values.
        // Some code uses all 64 bits. class_addIvar() over-allocates the
        // offset for their benefit.
#endif
        int32_t *offset;
        const char *name;
        const char *type;
        // alignment is sometimes -1; use alignment() instead
        uint32_t alignment_raw;
        uint32_t size;
        
        uint32_t alignment() const {
            if (alignment_raw == ~(uint32_t)0) return 1U << WORD_SHIFT;
            return 1 << alignment_raw;
        }
    };
    //------------------------------ivar64_t---------------------------------
    
    
    //------------------------------ivar64_list_t---------------------------------
    typedef struct ivar64_t *Ivar64;
    struct ivar64_list_t : entsize64_list_tt<ivar64_t, ivar64_list_t, 0> {
        bool containsIvar(Ivar64 ivar) const {
            return (ivar >= (Ivar64)&*begin()  &&  ivar < (Ivar64)&*end());
        }
    };
    //------------------------------ivar64_list_t---------------------------------
    
    
    //------------------------------property64_t---------------------------------
    struct property64_t {
        const char *name;
        const char *attributes;
    };
    //------------------------------property64_t---------------------------------
    
    
    //------------------------------property64_list_t---------------------------------
    struct property64_list_t : entsize64_list_tt<property64_t, property64_list_t, 0> {
    };
    //------------------------------property64_list_t---------------------------------
    
    
    //------------------------------protocol64_list_t---------------------------------
    typedef uintptr_t protocol_ref_t;  // protocol_t *, but unremapped
    struct protocol64_list_t {
        // count is 64-bit by accident.
        uintptr_t count;
        protocol_ref_t list[0]; // variable-size
        
        size_t byteSize() const {
            return sizeof(*this) + count*sizeof(list[0]);
        }
        
        protocol64_list_t *duplicate() const {
            return (protocol64_list_t *)memdup(this, this->byteSize());
        }
        
        typedef protocol_ref_t* iterator;
        typedef const protocol_ref_t* const_iterator;
        
        const_iterator begin() const {
            return list;
        }
        iterator begin() {
            return list;
        }
        const_iterator end() const {
            return list + count;
        }
        iterator end() {
            return list + count;
        }
    };
    //------------------------------protocol64_list_t---------------------------------
    
    
    struct objc_class64;
    typedef struct objc_class64 *Class64;
    //------------------------------isa64_t---------------------------------
    union isa64_t {
        isa64_t() { }
        isa64_t(uintptr_t value) : bits(value) { }
        
        Class64 cls;
        uintptr_t bits;
#if defined(ISA_BITFIELD)
        struct {
            ISA_BITFIELD;  // defined in isa.h
        };
#endif
    };
    //------------------------------isa64_t---------------------------------
    
    
    //------------------------------objc_object64---------------------------------
    struct objc_object64 {
    public:
        isa64_t isa;
        
    public:
        
        // ISA() assumes this is NOT a tagged pointer object
        Class64 ISA();
        
        // getIsa() allows this to be a tagged pointer object
        Class64 getIsa();
        
        bool isTaggedPointer();
        
        bool isBasicTaggedPointer();
        
        bool isExtTaggedPointer();
    };
    //------------------------------objc_object64---------------------------------
    
    
    //------------------------------class64_ro_t---------------------------------
    struct class64_ro_t {
        uint32_t flags;
        uint32_t instanceStart;
        uint32_t instanceSize;
#ifdef __LP64__
        uint32_t reserved;
#endif
        
        const uint8_t * ivarLayout;
        
        const char * name;
        method64_list_t * baseMethodList;
        protocol64_list_t * baseProtocols;
        const ivar64_list_t * ivars;
        
        const uint8_t * weakIvarLayout;
        property64_list_t *baseProperties;
        
        method64_list_t *baseMethods() const {
            return baseMethodList;
        }
        bool isMetaClass() const {
            return (flags & 0x1) == 1;
        }
        const char *getName() const {
            return (const char *)name;
        }
        std::string getDebugDescription(int level = 0) const;
    };
    //------------------------------class64_ro_t---------------------------------
    
    
    //------------------------------method64_array_t---------------------------------
    class method64_array_t :
    public list64_array_tt<method64_t, method64_list_t>
    {
        typedef list64_array_tt<method64_t, method64_list_t> Super;
        
    public:
        method64_list_t **beginCategoryMethodLists() {
            return beginLists();
        }
        
        method64_list_t **endCategoryMethodLists(Class64 cls);
        
        method64_array_t duplicate() {
            return Super::duplicate<method64_array_t>();
        }
    };
    //------------------------------method64_array_t---------------------------------
    
    
    //------------------------------property64_array_t---------------------------------
    class property64_array_t :
    public list64_array_tt<property64_t, property64_list_t>
    {
        typedef list64_array_tt<property64_t, property64_list_t> Super;
        
    public:
        property64_array_t duplicate() {
            return Super::duplicate<property64_array_t>();
        }
    };
    //------------------------------property64_array_t---------------------------------
    
    
    //------------------------------protocol64_array_t---------------------------------
    class protocol64_array_t :
    public list64_array_tt<protocol_ref_t, protocol64_list_t>
    {
        typedef list64_array_tt<protocol_ref_t, protocol64_list_t> Super;
        
    public:
        protocol64_array_t duplicate() {
            return Super::duplicate<protocol64_array_t>();
        }
    };
    //------------------------------protocol64_array_t---------------------------------
    
    
    //------------------------------class64_rw_t---------------------------------
    struct class64_rw_t {
        // Be warned that Symbolication knows the layout of this structure.
        uint32_t flags;
        uint32_t version;
        
        /*const*/ class64_ro_t *ro;
        
        method64_array_t methods;
        property64_array_t properties;
        protocol64_array_t protocols;
        
        Class64 firstSubclass;
        Class64 nextSiblingClass;
        
        char *demangledName;
        
#if SUPPORT_INDEXED_ISA
        uint32_t index;
#endif
    };
    //------------------------------class64_rw_t---------------------------------
    
    
    //------------------------------class64_data_bits_t---------------------------------
    struct class64_data_bits_t {
        
        // Values are the FAST_ flags above.
        uintptr_t bits;
    private:
        bool getBit(uintptr_t bit)
        {
            return bits & bit;
        }
        
        void setBits(uintptr_t set)
        {
            bits |= set;
        }
        
    public:
        
        class64_rw_t* data() const {
            return (class64_rw_t *)(bits & FAST_DATA_MASK);
        }
        void setData(class64_rw_t *newData)
        {
            assert(!data()  ||  (newData->flags & (RW_REALIZING | RW_FUTURE)));
            // Set during realization or construction only. No locking needed.
            // Use a store-release fence because there may be concurrent
            // readers of data and data's contents.
            uintptr_t newBits = (bits & ~FAST_DATA_MASK) | (uintptr_t)newData;
            bits = newBits;
        }
    };
    //------------------------------class64_data_bits_t---------------------------------
    
    
    typedef uint32_t mask_t;  // x86_64 & arm64 asm are less efficient with 16-bits
    typedef uintptr_t cache_key_t;
    //------------------------------bucket64_t---------------------------------
    using MethodCacheIMP = IMP;
    struct bucket64_t {
        MethodCacheIMP _imp;
        cache_key_t _key;
    };
    //------------------------------bucket64_t---------------------------------
    
    
    //------------------------------cache64_t---------------------------------
    struct cache64_t {
        struct bucket64_t *_buckets;
        mask_t _mask;
        mask_t _occupied;
    };
    //------------------------------cache64_t---------------------------------
    
    
    //------------------------------objc_class64---------------------------------
    struct objc_class64 : objc_object64 {
        // Class64 ISA;
        Class64 superclass;
        cache64_t cache;             // formerly cache pointer and vtable
        
        class64_data_bits_t bits;    // class64_rw_t * plus custom rr/alloc flags
        
        class64_rw_t *data() const {
            return bits.data();
        }
        objc_class64 *getIsaPointer() const {
            return isa.cls;
        }
        objc_class64 *getSuperclassPointer() const {
            return superclass->isa.cls;
        }
        
        class64_rw_t *getRwDataPointer() const {
            return data();
        }
        
        class64_ro_t *getRoDataPointer() const {
            return (class64_ro_t *)bits.bits;
        }
        
        std::string getDebugDescription(int level = 0) const;
    };
    //------------------------------objc_class64---------------------------------
    
}}

#endif /* objc_types_h */
