//
//  ZQObjCSectionRuntimeRegister.h
//  ZQJITEngine
//
//  Created by zuqingxie on 2019/4/12.
//  Copyright © 2019年 zqx. All rights reserved.
//

#ifndef ZQObjCSectionRuntimeRegister_h
#define ZQObjCSectionRuntimeRegister_h

#include <vector>
#include "ZQObjCSectionHandlerBase.h"

namespace zq { namespace jit {
        
    class ZQObjCSectionRuntimeRegister {
    private:
        using SectionList = std::unique_ptr<std::vector<Section>>;
        SectionList _currentModuleSectionList;
        std::vector<SectionList> _moduleSectionList;
        
        using SectionHandler = std::unique_ptr<ZQObjCSectionHandlerBase>;
        std::vector<SectionHandler> _handlerList;
    public:
        ZQObjCSectionRuntimeRegister();
        ~ZQObjCSectionRuntimeRegister();
        
        void addObjCSetion(Section &section);
        void registerObjCSections();
    };
}}

#endif /* ZQObjCSectionRuntimeRegister_h */
