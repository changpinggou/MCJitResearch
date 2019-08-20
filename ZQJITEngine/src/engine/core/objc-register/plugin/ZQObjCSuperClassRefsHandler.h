//
//  ZQObjCSuperClassRefsHandler.h
//  ZQJITEngine
//
//  Created by zuqingxie on 2019/4/15.
//  Copyright © 2019年 zqx. All rights reserved.
//

#ifndef ZQObjCSuperClassRefsHandler_h
#define ZQObjCSuperClassRefsHandler_h

#include "ZQObjCSectionHandlerBase.h"

namespace zq { namespace jit {
    
    class ZQObjCSuperClassRefsHandler : public ZQObjCSectionHandlerBase {
    public:
        virtual ~ZQObjCSuperClassRefsHandler();
        virtual bool registerSection(const Section &section) override;
    };
    
}}

#endif /* ZQObjCSuperClassRefsHandler_h */
