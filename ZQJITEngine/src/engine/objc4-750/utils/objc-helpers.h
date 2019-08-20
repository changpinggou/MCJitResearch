//
//  objc-helpers.h
//  ZQJITEngine
//
//  Created by zuqingxie on 2019/4/9.
//  Copyright © 2019年 zqx. All rights reserved.
//

#ifndef objc_helpers_h
#define objc_helpers_h

namespace zq { namespace objc {
    bool objc_classIsRegistered(void *clz); //void *->Class
    void objc_dumpClass(void *clz); //void *->Class
    void objc_dumpClasses();
}}

#endif /* objc_helpers_h */
