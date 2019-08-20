//
//  ZQNewObjCClassHandler.h
//  ZQJITEngine
//
//  Created by zuqingxie on 2019/4/12.
//  Copyright © 2019年 zqx. All rights reserved.
//

#ifndef ZQNewObjCClassHandler_h
#define ZQNewObjCClassHandler_h

#include "ZQObjCSectionHandlerBase.h"
#include <vector>
#include "objc-types.h"

namespace zq { namespace jit {
    
    class ZQNewObjCClassHandler : public ZQObjCSectionHandlerBase {
    public:
        virtual ~ZQNewObjCClassHandler();
        virtual bool registerSection(const Section &section) override;
        void registerNewClass(zq::objc::Class64 newClass, std::string superClassName, bool superClsIsDynamic);
        void findClassDependency(std::vector<zq::objc::Class64> &dependencyClasses, std::vector<zq::objc::Class64> &allClasses);
        zq::objc::Class64 findAndRemoveClass(std::vector<zq::objc::Class64> &allClasses, const char *className);
    };
    
}}

#endif /* ZQNewObjCClassHandler_h */
