//
//  ZQObjCSectionHandlerBase.h
//  ZQJITEngine
//
//  Created by zuqingxie on 2019/4/12.
//  Copyright © 2019年 zqx. All rights reserved.
//

#ifndef ZQObjCSectionHandlerBase_h
#define ZQObjCSectionHandlerBase_h

#include <llvm/ADT/StringRef.h>

namespace zq { namespace jit {
    
    struct Section {
        uint8_t    *_ptr;
        uint32_t   _size;
        llvm::StringRef _name;
        Section(uint8_t *ptr, uint32_t size, llvm::StringRef name): _ptr(ptr), _size(size), _name(name){};
    };
    
    class ZQObjCSectionHandlerBase {
    public:
        virtual ~ZQObjCSectionHandlerBase();
        virtual bool registerSection(const Section &section);
    };
    
}}

#endif /* ZQObjCSectionHandlerBase_h */
