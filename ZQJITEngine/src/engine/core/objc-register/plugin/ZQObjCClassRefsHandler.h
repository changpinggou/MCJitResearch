//
//  ZQObjCClassRefsHandler.h
//  ZQJITEngine
//
//  Created by zuqingxie on 2019/4/15.
//  Copyright © 2019年 zqx. All rights reserved.
//

#ifndef ZQObjCClassRefsHandler_h
#define ZQObjCClassRefsHandler_h

#include "ZQObjCSectionHandlerBase.h"

namespace zq { namespace jit {
    
    class ZQObjCClassRefsHandler : public ZQObjCSectionHandlerBase {
    public:
        virtual ~ZQObjCClassRefsHandler();
        virtual bool registerSection(const Section &section) override;
        void registerClassRefsSection(const Section &section);
    };
    
}}

#endif /* ZQObjCClassRefsHandler_h */
