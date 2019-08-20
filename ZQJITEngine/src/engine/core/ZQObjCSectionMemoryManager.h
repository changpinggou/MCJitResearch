//
//  ZQObjCSectionMemoryManager.h
//  ZQJITEngine
//
//  Created by zuqingxie on 2019/4/12.
//  Copyright © 2019年 zqx. All rights reserved.
//

#ifndef ZQObjCSectionMemoryManager_h
#define ZQObjCSectionMemoryManager_h

#include <llvm/ADT/StringRef.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include "ZQObjCSectionRuntimeRegister.h"

namespace zq { namespace jit {
    
    class ZQObjCSectionMemoryManager: public llvm::SectionMemoryManager {
    private:
        ZQObjCSectionRuntimeRegister _sectionRegister;
    public:
        uint8_t *allocateDataSection(uintptr_t Size, unsigned Alignment,
                                     unsigned SectionID, llvm::StringRef SectionName,
                                     bool isReadOnly) override;
        
        bool finalizeMemory(std::string *ErrMsg = nullptr) override;
    };
    
}}

#endif /* ZQObjCSectionMemoryManager_h */
