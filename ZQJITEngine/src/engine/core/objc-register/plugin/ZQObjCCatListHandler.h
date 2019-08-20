//
//  ZQObjCCatListHandler.h
//  ZQJITEngine
//
//  Created by zuqingxie on 2019/4/15.
//  Copyright © 2019年 zqx. All rights reserved.
//

#ifndef ZQObjCCatListHandler_h
#define ZQObjCCatListHandler_h

#include "ZQObjCSectionHandlerBase.h"

namespace zq { namespace jit {
    
    class ZQObjCCatListHandler : public ZQObjCSectionHandlerBase {
    public:
        virtual ~ZQObjCCatListHandler();
        virtual bool registerSection(const Section &section) override;
    };
    
}}

#endif /* ZQObjCCatListHandler_h */
