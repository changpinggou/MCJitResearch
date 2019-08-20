//
//  objc-helpers.cpp
//  ZQJITEngine
//
//  Created by zuqingxie on 2019/4/9.
//  Copyright © 2019年 zqx. All rights reserved.
//

#include "objc-helpers.h"
#include <iostream>
#include <assert.h>
#include <objc/runtime.h>

namespace zq { namespace objc {
    
    void objc_dumpClasses()
    {
        int numRegisteredClasses = objc_getClassList(NULL, 0);
        assert(numRegisteredClasses > 0);
        Class *classes = (Class *)malloc(sizeof(Class) * numRegisteredClasses);
        numRegisteredClasses = objc_getClassList(classes, numRegisteredClasses);
        for (int i = 0; i < numRegisteredClasses; i++) {
            std::cout << "registered class: " << class_getName(classes[i]) << "\n";
        }
        free(classes);
    }
    
    bool objc_classIsRegistered(void *clz)
    {
        Class cls = (Class)clz;
        int numRegisteredClasses = objc_getClassList(NULL, 0);
        assert(numRegisteredClasses > 0);
        Class *classes = (Class *)malloc(sizeof(Class) * numRegisteredClasses);
        numRegisteredClasses = objc_getClassList(classes, numRegisteredClasses);
        for (int i = 0; i < numRegisteredClasses; i++) {
            if (classes[i] == cls) {
                free(classes);
                return true;
            }
        }
        free(classes);
        return false;
    }
    
    void objc_dumpClass(void *clz)
    {
        Class cls = (Class)clz;
        printf("class_dumpMethods() dumping class: %p, is meta class: %d\n", (void *)cls, class_isMetaClass(cls));
        uint32_t methodCount = 0;
        Method *methods = class_copyMethodList(cls, &methodCount);
        printf("Found %d methods on '%s'\n", methodCount, class_getName(cls));
        
        for (uint32_t i = 0; i < methodCount; i++) {
            Method method = methods[i];
            printf("\t'%s' has method named '%s' of encoding '%s' %p\n", class_getName(cls), sel_getName(method_getName(method)), method_getTypeEncoding(method), (void *)method);
        }
        free(methods);
    }
    
}}
