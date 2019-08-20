//
//  ZQExistedObjCClassHandler.h
//  ZQJITEngine
//
//  Created by zuqingxie on 2019/4/12.
//  Copyright © 2019年 zqx. All rights reserved.
//

#ifndef ZQExistedObjCClassHandler_h
#define ZQExistedObjCClassHandler_h

#include "ZQObjCSectionHandlerBase.h"
#include "objc-types.h"

namespace zq { namespace jit {
    
    class ZQExistedObjCClassHandler : public ZQObjCSectionHandlerBase {
    public:
        virtual ~ZQExistedObjCClassHandler();
        virtual bool registerSection(const Section &section) override;
    private:
        bool modifyExsitedClass(zq::objc::Class64 oldClass, zq::objc::Class64 newClass);
        void modifyIvarAndProperty(zq::objc::Class64 oldClass, zq::objc::Class64 newClass);
        void modifyInstanceMethod(zq::objc::Class64 oldClass, zq::objc::Class64 newClass);
        void modifyClassMethod(zq::objc::Class64 oldClass, zq::objc::Class64 newClass);
    };
    
}}

#endif /* ZQExistedObjCClassHandler_h */
