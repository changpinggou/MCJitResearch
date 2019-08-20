//
//  ZQObjCSectionMemoryManager.cpp
//  ZQJITEngine
//
//  Created by zuqingxie on 2019/4/12.
//  Copyright © 2019年 zqx. All rights reserved.
//

#include "ZQObjCSectionMemoryManager.h"

namespace zq { namespace jit {
    
    uint8_t *ZQObjCSectionMemoryManager::allocateDataSection(uintptr_t Size, unsigned Alignment,
                                                             unsigned SectionID, llvm::StringRef SectionName,
                                                             bool isReadOnly)
    {
        uint8_t *sectionMemory = SectionMemoryManager::allocateDataSection(Size,
                                                                           Alignment,
                                                                           SectionID,
                                                                           SectionName,
                                                                           isReadOnly);
        zq::jit::Section section(sectionMemory, Size, SectionName);
        _sectionRegister.addObjCSetion(section);
        return sectionMemory;
    }
    
    bool ZQObjCSectionMemoryManager::finalizeMemory(std::string *ErrMsg)
    {
        _sectionRegister.registerObjCSections();
        return SectionMemoryManager::finalizeMemory(ErrMsg);
    }
    
}}
