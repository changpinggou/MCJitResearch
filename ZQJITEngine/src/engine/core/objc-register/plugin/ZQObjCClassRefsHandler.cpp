//
//  ZQObjCClassRefsHandler.cpp
//  ZQJITEngine
//
//  Created by zuqingxie on 2019/4/15.
//  Copyright © 2019年 zqx. All rights reserved.
//

#include "ZQObjCClassRefsHandler.h"
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/raw_ostream.h>
#include "objc-types.h"
#include <objc/runtime.h>

using namespace llvm;
using namespace zq::objc;

namespace zq { namespace jit {
    
    ZQObjCClassRefsHandler::~ZQObjCClassRefsHandler()
    {
        
    }
    
    bool ZQObjCClassRefsHandler::registerSection(const Section &section)
    {
        if (section._name.find("__objc_classrefs") != StringRef::npos) {
            registerClassRefsSection(section);
        }
        return true;
    }
    
    void ZQObjCClassRefsHandler::registerClassRefsSection(const Section &section)
    {
        Class64 *classRefs = (Class64 *)section._ptr;
        for (uint32_t i = 0; i < section._size / 16; i++) {
            Class64 *classRefPtr = &classRefs[i];
            const char *className = object_getClassName((id)*classRefPtr);
            assert(className);
            if (!className || 0 == strcmp(className, "nil")) {
                continue;
            }
            Class newClz = objc_getRequiredClass(className);
            assert(class_isMetaClass(newClz) == false);
            outs() << "ZQObjCClassRefsHandler> find class ref " << className << ", class address: " << (void *)newClz << "\n";
            
            if (*classRefPtr != (Class64)newClz) {
                *classRefPtr = (Class64)objc_getRequiredClass(className);
            }
        }
    }
}}
