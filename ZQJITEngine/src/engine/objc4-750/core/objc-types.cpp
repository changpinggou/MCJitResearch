//
//  objc-types.cpp
//  ZQJITEngine
//
//  Created by zuqingxie on 2019/4/8.
//  Copyright © 2019年 zqx. All rights reserved.
//

#include "objc-types.h"
#include <sstream>
#include "objc-helpers.h"

namespace zq { namespace objc {
    
    std::string method64_t::getDebugDescription(int level) const
    {
        std::ostringstream os;
        std::string padding = std::string(level * 4, ' ');
        
        os << padding << "[method_t]\n";
        os << padding << "\t" << "name: " << sel_getName(name) << "\n";
        os << padding << "\t" << "types: " << (const char *)types << "\n";
        os << padding << "\t" << "imp: " <<  (const void * const *)&imp << " / " << (const void *)imp << " / " << "\n";
        
        return os.str();
    }
    
    std::string method64_list_t::getDebugDescription(int level) const
    {
        std::ostringstream os;
        std::string padding = std::string(level * 4, ' ');
        const uint32_t entsize = this->entsize();
        if (entsize == 0) {
            os << padding << "[error]" << "\n";
            return os.str();
        }
        
        os << padding << "[method_list_t]\n";
        os << padding << "entsize: " << entsize << "\n";
        os << padding << "count: " << count << "\n";
        for (uint32_t i = 0; i < count; i++) {
            const method64_t method = get(i);
            os << method.getDebugDescription(level + 1);
        }
        
        return os.str();
    }
    
    std::string class64_ro_t::getDebugDescription(int level) const
    {
        std::ostringstream os;
        std::string padding = std::string(level * 4, ' ');
        
        os << padding << "[class_ro_t] (metaclass: " << (isMetaClass() ? "yes" : "no") << ")\n";
        os << padding << "name: " << getName() << "\n";
        os << padding << "baseMethods: " << (void *)baseMethods() << "\n";
        if (method64_list_t *methodListPointer = baseMethods()) {
            os << methodListPointer->getDebugDescription(level + 1);
        }
        return os.str();
    }
    
    Class64
    objc_object64::ISA()
    {
        assert(!isTaggedPointer());
#if SUPPORT_INDEXED_ISA
        if (isa.nonpointer) {
            uintptr_t slot = isa.indexcls;
            return classForIndex((unsigned)slot);
        }
        return (Class64)isa.bits;
#else
        return (Class64)(isa.bits & ISA_MASK);
#endif
    }
    
    Class64
    objc_object64::getIsa()
    {
        if (!isTaggedPointer()) return ISA();
        
        uintptr_t ptr = (uintptr_t)this;
        if (isExtTaggedPointer()) {
            uintptr_t slot =
            (ptr >> _OBJC_TAG_EXT_SLOT_SHIFT) & _OBJC_TAG_EXT_SLOT_MASK;
            return (Class64)objc_tag_ext_classes[slot];
        } else {
            uintptr_t slot =
            (ptr >> _OBJC_TAG_SLOT_SHIFT) & _OBJC_TAG_SLOT_MASK;
            return (Class64)objc_tag_classes[slot];
        }
    }
    
    
    inline bool
    objc_object64::isTaggedPointer()
    {
        return _objc_isTaggedPointer(this);
    }
    
    inline bool
    objc_object64::isBasicTaggedPointer()
    {
        return isTaggedPointer()  &&  !isExtTaggedPointer();
    }
    
    inline bool
    objc_object64::isExtTaggedPointer()
    {
        uintptr_t ptr = _objc_decodeTaggedPointer(this);
        return (ptr & _OBJC_TAG_EXT_MASK) == _OBJC_TAG_EXT_MASK;
    }
    
    std::string objc_class64::getDebugDescription(int level) const
    {
        std::ostringstream os;
        std::string padding = std::string(level * 4, ' ');
        
        os << padding << "[objc_class]" << "\n";
        os << padding << "this: " << (void *)this << " / " << "\n";
        os << padding << "isa: " << (void *)isa.bits << "\n";
        if (level == 0) {
            os << getIsaPointer()->getDebugDescription(1);
        }
        os << padding << "superclass: " << (void *const *)&superclass << "/" << (void *)superclass << "\n";
        if (level == 1) {
            Class superClz = (Class)getSuperclassPointer();
            if (objc_classIsRegistered((void *)superClz)) {
                os << padding << "\t(registered) " << object_getClassName((id)superClz) << "\n";
            } else {
                objc_class64 *superclassPointer = getSuperclassPointer();
                os << superclassPointer->getDebugDescription(1);
            }
        }
        
        os << padding << "data: " << (void *const *)&bits << "/" << (void *)bits.bits << "\n";
        
        class64_ro_t *dataPtr = getRoDataPointer();
        os << dataPtr->getDebugDescription(level + 1);
        
        return os.str();
    }

}}
