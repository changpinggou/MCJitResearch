//
//  ZQObjCSelRefsHandler.cpp
//  ZQJITEngine
//
//  Created by zuqingxie on 2019/4/12.
//  Copyright © 2019年 zqx. All rights reserved.
//

#include "ZQObjCSelRefsHandler.h"
#include <llvm/ADT/StringRef.h>
#include <objc/objc.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

namespace zq { namespace jit {
    
    ZQObjCSelRefsHandler::~ZQObjCSelRefsHandler()
    {
        
    }
    
    bool ZQObjCSelRefsHandler::registerSection(const Section &section)
    {
        if (section._name.find("__objc_selrefs") != StringRef::npos) {
            return registerSelRefsSection(section);
        }
        return true;
    }
    
    bool  ZQObjCSelRefsHandler::registerSelRefsSection(const Section &section)
    {
        uint8_t *sectionStart = section._ptr;
        uint32_t sectionSize = section._size;
        for (uint8_t *cursor = sectionStart; cursor < (sectionStart + sectionSize); cursor = cursor + sizeof(SEL)) {
            SEL *selector = (SEL *)cursor;
            if (!(*selector)) {
                continue;
            }
            const char *name = sel_getName(*selector);
            outs() << "ZQObjCSelRefsHandler> trying to register selector: " << *selector << "/" << name << "\n";
            *selector = sel_registerName(name);
            outs() << "ZQObjCSelRefsHandler> register selector :" << *selector << "complete!\n";
        }
        return true;
    }
    
}}
