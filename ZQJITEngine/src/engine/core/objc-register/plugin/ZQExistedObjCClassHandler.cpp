//
//  ZQExistedObjCClassHandler.cpp
//  ZQJITEngine
//
//  Created by zuqingxie on 2019/4/12.
//  Copyright © 2019年 zqx. All rights reserved.
//

#include "ZQExistedObjCClassHandler.h"
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/raw_ostream.h>
#include <objc/runtime.h>
#include <vector>

using namespace llvm;
using namespace zq::objc;

namespace zq { namespace jit {
    
    ZQExistedObjCClassHandler::~ZQExistedObjCClassHandler()
    {
        
    }
    
    bool ZQExistedObjCClassHandler::registerSection(const Section &section)
    {
        if (section._name.find("__objc_classlist") == StringRef::npos) {
            return true;
        }
        
        uint8_t *sectionPtr = section._ptr;
        uint32_t sectionSize = section._size;
        
        Class64 *classes = (Class64 *)sectionPtr;
        for (uintptr_t i = 0; i < sectionSize / 16; i += 1) {
            Class64 *classPointer = &classes[i];
            if (*classPointer == NULL) {
                continue;
            }
            Class64 newClass = *classPointer;
            const char *className = newClass->getRoDataPointer()->getName();
            outs() << "ZQExistedObjCClassHandler> find class: " << className << "\n";
            Class oldClass = objc_getClass(className);
            if (oldClass) {
                modifyExsitedClass((Class64)oldClass, newClass);
            } else {
                errs() << "ZQExistedObjCClassHandler> class is not existed: " << className << "\n";
            }
        }
        return true;
    }
    
    bool ZQExistedObjCClassHandler::modifyExsitedClass(Class64 oldClass, Class64 newClass)
    {
        if (!oldClass || !newClass) {
            return true;
        }
        modifyIvarAndProperty(oldClass, newClass);
        modifyInstanceMethod(oldClass, newClass);
        modifyClassMethod(oldClass, newClass);
        return true;
    }
    
    void ZQExistedObjCClassHandler::modifyIvarAndProperty(Class64 oldClass, Class64 newClass)
    {
        if (!oldClass || !newClass) {
            return;
        }
        class64_ro_t *newClassRo = newClass->getRoDataPointer();
        if(!newClassRo || !newClassRo->ivars) {
            class64_ro_t *oldClassRo = oldClass->getRwDataPointer()->ro;
            oldClassRo->ivars = nullptr;
            oldClassRo->instanceSize = oldClassRo->instanceStart;
            return;
        }
        class64_ro_t *oldClassRo = oldClass->getRwDataPointer()->ro;
        oldClassRo->ivarLayout = newClassRo->ivarLayout;
        oldClassRo->weakIvarLayout = newClassRo->weakIvarLayout;
        // 计算变量偏移，继承父类的内存布局
        uint32_t isaOffset = sizeof(isa64_t);
        for (uint32_t i = 0; i < newClassRo->ivars->count; i++) {
            ivar64_t ivar = newClassRo->ivars->get(i);
            *ivar.offset = (*ivar.offset - isaOffset) + oldClassRo->instanceStart;
        }
        oldClassRo->instanceSize = oldClassRo->instanceStart + newClassRo->instanceSize - isaOffset;
        oldClassRo->ivars = newClassRo->ivars;
        
        // 注册property
        if (newClassRo->baseProperties) {
            oldClassRo->baseProperties = newClassRo->baseProperties;
            outs() << "ZQExistedObjCClassHandler> replace " << oldClassRo->baseProperties->count << " propertys\n";
        }
    }
    
    void ZQExistedObjCClassHandler::modifyInstanceMethod(Class64 oldClass, Class64 newClass)
    {
        // 注册实例方法
        if (!oldClass || !newClass) {
            return;
        }
        class64_ro_t *newClassRo = newClass->getRoDataPointer();
        if (!newClassRo || !newClassRo->baseMethods()) {
            return;
        }
        uint32_t orgMethodCount = 0;
        method64_t **orgMethodList = (method64_t **)class_copyMethodList((Class)oldClass, &orgMethodCount);
        std::vector<method64_t *> orgMethodArray;
        for (uint32_t i = 0; i < orgMethodCount; ++i) {
            orgMethodArray.push_back(orgMethodList[i]);
        }
        for (uint32_t i = 0; i < newClassRo->baseMethods()->count; i++) {
            const method64_t newMethod = newClassRo->baseMethods()->get(i);
            const char *name = sel_getName(newMethod.name);
            if (!name) {
                continue;
            }
            SEL selector = sel_registerName(name);
            class_replaceMethod((Class)oldClass, selector, (IMP)newMethod.imp, newMethod.types);
            outs() << "ZQExistedObjCClassHandler> replace instance method: " << name << "\n";
            for (auto methodIter = orgMethodArray.begin(); methodIter != orgMethodArray.end(); ++methodIter) {
                if (0 == strcmp(sel_getName(newMethod.name), sel_getName((*methodIter)->name)) && 0 == strcmp(newMethod.types, (*methodIter)->types)) {
                    orgMethodArray.erase(methodIter);
                    break;
                }
            }
        }
        // 将删除的方法设置为父类的IMP，或者如果父类未实现，则设置为objc_msgForward指针
        Class superClass = class_getSuperclass((Class)oldClass);
        for (auto methodIter = orgMethodArray.begin(); methodIter != orgMethodArray.end(); ++methodIter) {
            const char *name = sel_getName((*methodIter)->name);
            SEL selector = sel_registerName(name);
            IMP superImp = nullptr;
            method64_t *superMethod = (method64_t *)class_getInstanceMethod(superClass, selector);
            if (!superMethod) {
                superImp = class_getMethodImplementation(superClass, selector);
                outs() << "ZQExistedObjCClassHandler> delete method: " << name << " super method imp: NULL\n";
            } else {
                superImp = superMethod->imp;
                outs() << "ZQExistedObjCClassHandler> delete method: " << name << " super method imp: " << (void *)superImp << "\n";
            }
            class_replaceMethod((Class)oldClass, selector, superImp, (*methodIter)->types);
        }
        if (orgMethodList) {
            free(orgMethodList);
        }
    }
    
    void ZQExistedObjCClassHandler::modifyClassMethod(Class64 oldClass, Class64 newClass)
    {
        if (!oldClass || !newClass) {
            return;
        }
        Class64 newMetaClass = newClass->getIsa();
        if (!newMetaClass || !newMetaClass->getRoDataPointer()) {
            return;
        }
        const Class oldMetaClass = objc_getMetaClass(newClass->getRoDataPointer()->getName());
        uint32_t orgMethodCount = 0;
        method64_t **orgMethodList = (method64_t **)class_copyMethodList(oldMetaClass, &orgMethodCount);
        std::vector<method64_t *> orgMethodArray;
        for (uint32_t i = 0; i < orgMethodCount; ++i) {
            orgMethodArray.push_back(orgMethodList[i]);
        }
        // 注册类方法
        method64_list_t *baseMethods = newMetaClass->getRoDataPointer()->baseMethods();
        for (uint32_t i = 0; baseMethods && i < baseMethods->count; i++) {
            const method64_t newMethod = (baseMethods->get(i));
            const char *name = sel_getName(newMethod.name);
            if (!name) {
                continue;
            }
            SEL selector = sel_registerName(name);
            class_replaceMethod(oldMetaClass, selector, (IMP)newMethod.imp, newMethod.types);
            outs() << "ZQExistedObjCClassHandler> replace class method: " << name << "\n";
            for (auto methodIter = orgMethodArray.begin(); methodIter != orgMethodArray.end(); ++methodIter) {
                if (0 == strcmp(sel_getName(newMethod.name), sel_getName((*methodIter)->name)) && 0 == strcmp(newMethod.types, (*methodIter)->types)) {
                    orgMethodArray.erase(methodIter);
                    break;
                }
            }
        }
        // 将删除的方法设置为父类的IMP，或者如果父类未实现，则设置为objc_msgForward指针
        Class superMetaClass = class_getSuperclass(oldMetaClass);
        for (auto methodIter = orgMethodArray.begin(); methodIter != orgMethodArray.end(); ++methodIter) {
            const char *name = sel_getName((*methodIter)->name);
            SEL selector = sel_registerName(name);
            IMP superImp = nullptr;
            method64_t *superClassMethod = (method64_t *)class_getInstanceMethod(superMetaClass, selector);
            if (!superClassMethod) {
                superImp = class_getMethodImplementation(superMetaClass, selector);
                outs() << "ZQExistedObjCClassHandler> delete class method: " << name << " super class method imp: NULL\n";
            } else {
                superImp = superClassMethod->imp;
                outs() << "ZQExistedObjCClassHandler> delete class method: " << name << " super class method imp: " << (void *)superImp << "\n";
            }
            class_replaceMethod(oldMetaClass, selector, superImp, (*methodIter)->types);
        }
        if (orgMethodList) {
            free(orgMethodList);
        }
    }
    
}}
