//
//  ZQNewObjCClassHandler.cpp
//  ZQJITEngine
//
//  Created by zuqingxie on 2019/4/12.
//  Copyright © 2019年 zqx. All rights reserved.
//

#include "ZQNewObjCClassHandler.h"
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/raw_ostream.h>
#include <objc/runtime.h>

using namespace llvm;
using namespace zq::objc;

extern "C" Class objc_readClassPair(Class clazz, const struct objc_image_info *info);

namespace zq { namespace jit {
    
    static bool objc_classIsRegistered(Class clazz)
    {
        int numRegisteredClasses = objc_getClassList(NULL, 0);
        Class *classes = (Class *)malloc(sizeof(Class) * numRegisteredClasses);
        numRegisteredClasses = objc_getClassList(classes, numRegisteredClasses);
        for (int i = 0; i < numRegisteredClasses; i++) {
            if (classes[i] == clazz) {
                free(classes);
                return true;
            }
        }
        free(classes);
        return false;
    }
    
    static std::string getSuperClassName(Class64 clazz)
    {
        if (!clazz || !clazz->superclass) {
            return "";
        }
        Class64 superClass = clazz->superclass;
        if (objc_classIsRegistered((Class)superClass)) {
            return clazz->superclass->getRwDataPointer()->ro->getName();
        } else {
            return clazz->superclass->getRoDataPointer()->getName();
        }
    }
    
    ZQNewObjCClassHandler::~ZQNewObjCClassHandler()
    {
        
    }
    
    /* 目前只支持新增一级类，即支持新增的类的父类为已有的类，该父类是app编译后或系统自带的，而不是先前或者本次注入的新类
       fix me: 以后支持二级或多级类继承
     */
    bool ZQNewObjCClassHandler::registerSection(const Section &section)
    {
        if (section._name.find("__objc_classlist") == StringRef::npos) {
            return true;
        }
        
        std::vector<std::string> allClassesName; //  模块里所有的类
        std::vector<Class64> newClasses;         // 目前不存在的类
        uint8_t *sectionPtr = section._ptr;
        uint32_t sectionSize = section._size;
        Class64 *classes = (Class64 *)sectionPtr;
        for (uintptr_t i = 0; i < sectionSize / 16; ++i) {
            Class64 *classPointer = &classes[i];
            if (*classPointer == NULL) {
                continue;
            }
            Class64 newClass = *classPointer;
            const char *className = newClass->getRoDataPointer()->getName();
            allClassesName.push_back(className);
            if (!objc_getClass(className)) {
                outs() << "ZQNewObjCClassHandler> new class: " << className << "\n";
                newClasses.push_back((Class64)newClass);
            }
        }
        
        std::vector<Class64> toBeRegisteredClasses;
        const size_t newClassCount = newClasses.size();
        while (newClassCount > 0 && toBeRegisteredClasses.size() < newClassCount) {
            std::vector<Class64> dependencyClasses; // 依赖的类，假设A->B，则这个数组的顺序为B、A
            findClassDependency(dependencyClasses, newClasses);
            for (auto classIter = dependencyClasses.rbegin(); classIter != dependencyClasses.rend(); ++classIter) {
                toBeRegisteredClasses.push_back(*classIter);
            }
            if (dependencyClasses.empty() || newClasses.empty()) {
                break;
            }
        }
        for (auto newClass : toBeRegisteredClasses) {
            std::string superClassName = getSuperClassName(newClass);
            // 某个类的父类是否也为注入的类，如果父类不在模块内，则父类已经注册了，否则父类在当前模块内，则父类称之为动态类
            bool superClassIsDynamic = false;
            for (auto className : allClassesName) {
                if (className == superClassName) {
                    superClassIsDynamic = true;
                }
            }
            registerNewClass(newClass, superClassName, superClassIsDynamic);
        }
        return true;
    }
    
    void ZQNewObjCClassHandler::findClassDependency(std::vector<Class64> &dependencyClasses, std::vector<Class64> &allClasses)
    {
        if (allClasses.empty()) {
            return;
        }
        if (dependencyClasses.empty()) {
            dependencyClasses.push_back(allClasses.front());
            allClasses.erase(allClasses.begin());
        }
        Class64 rootClass = dependencyClasses.back();
        if (objc_classIsRegistered((Class)(rootClass->superclass)) || allClasses.empty()) {
            return;
        }
        const char *superClassName = rootClass->superclass->getRoDataPointer()->getName();
        Class64 superClass = findAndRemoveClass(allClasses, superClassName);
        assert(superClass);
        dependencyClasses.push_back(superClass);
        findClassDependency(dependencyClasses, allClasses);
    }
    
    Class64 ZQNewObjCClassHandler::findAndRemoveClass(std::vector<Class64> &allClasses, const char *className)
    {
        for (auto classIter = allClasses.begin(); classIter != allClasses.end(); ++classIter) {
            Class64 clazz = *classIter;
            if (0 == strcmp(className, clazz->getRoDataPointer()->getName())) {
                allClasses.erase(classIter);
                return clazz;
            }
        }
        return nullptr;
    }
    
    void ZQNewObjCClassHandler::registerNewClass(Class64 newClass, std::string superClassName, bool superClassIsDynamic)
    {
        if (!newClass || !newClass->superclass || 0 == superClassName.length()) {
            return;
        }
        outs() << "ZQNewObjCClassHandler> register class: " << newClass->getRoDataPointer()->getName() << " super class: " << superClassName << " super class is dynamic: " << superClassIsDynamic << "\n";
        if (superClassIsDynamic) {
            newClass->superclass = (Class64)objc_getClass(superClassName.c_str());
            newClass->superclass->isa.cls = (Class64)objc_getMetaClass(superClassName.c_str());
        }
        Class runtimeClass = objc_readClassPair((Class)newClass, NULL);
        assert(runtimeClass);
    }
    
}}
