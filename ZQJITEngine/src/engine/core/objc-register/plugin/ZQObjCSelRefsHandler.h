//
//  ZQObjCSelRefsHandler.h
//  ZQJITEngine
//
//  Created by zuqingxie on 2019/4/12.
//  Copyright © 2019年 zqx. All rights reserved.
//

#ifndef ZQObjCSelRefsHandler_h
#define ZQObjCSelRefsHandler_h

#include "ZQObjCSectionHandlerBase.h"

namespace zq { namespace jit {
    
    class ZQObjCSelRefsHandler : public ZQObjCSectionHandlerBase {
    public:
        virtual ~ZQObjCSelRefsHandler();
        virtual bool registerSection(const Section &section) override;
    private:
        bool registerSelRefsSection(const Section &section);
    };
    
}}

#endif /* ZQObjCSelRefsHandler_h */
