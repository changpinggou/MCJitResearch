//
//  ZQObjCSectionRuntimeRegister.cpp
//  ZQJITEngine
//
//  Created by zuqingxie on 2019/4/12.
//  Copyright © 2019年 zqx. All rights reserved.
//

#include "ZQObjCSectionRuntimeRegister.h"
#include <llvm/Support/raw_ostream.h>
#include "ZQObjCSelRefsHandler.h"
#include "ZQExistedObjCClassHandler.h"
#include "ZQNewObjCClassHandler.h"
#include "ZQObjCClassRefsHandler.h"
#include "ZQObjCSuperClassRefsHandler.h"
#include "ZQObjCCatListHandler.h"

using namespace llvm;

namespace zq { namespace jit {
    
    ZQObjCSectionRuntimeRegister::ZQObjCSectionRuntimeRegister() :
    _currentModuleSectionList(nullptr)
    {
        _handlerList.push_back(std::unique_ptr<ZQObjCSectionHandlerBase>(new ZQObjCSelRefsHandler));
        _handlerList.push_back(std::unique_ptr<ZQObjCSectionHandlerBase>(new ZQExistedObjCClassHandler));
        _handlerList.push_back(std::unique_ptr<ZQObjCSectionHandlerBase>(new ZQNewObjCClassHandler));
        _handlerList.push_back(std::unique_ptr<ZQObjCSectionHandlerBase>(new ZQObjCClassRefsHandler));
        _handlerList.push_back(std::unique_ptr<ZQObjCSectionHandlerBase>(new ZQObjCSuperClassRefsHandler));
        _handlerList.push_back(std::unique_ptr<ZQObjCSectionHandlerBase>(new ZQObjCCatListHandler));
    }
    
    ZQObjCSectionRuntimeRegister::~ZQObjCSectionRuntimeRegister()
    {
        
    }
    
    void ZQObjCSectionRuntimeRegister::addObjCSetion(Section &section)
    {
        if (section._name.find("objc") != llvm::StringRef::npos) {
            outs() << "ObjCSectionRuntimeRegister::addObjCSection: "
            << section._name << " "
            << "addr: " << section._ptr << " "
            << "size: " << section._size
            << "\n";
            if (!_currentModuleSectionList) {
                _currentModuleSectionList.reset(new std::vector<Section>);
            }
            _currentModuleSectionList->push_back(section);
        }
    }
    
    void ZQObjCSectionRuntimeRegister::registerObjCSections()
    {
        if (!_currentModuleSectionList) {
            return;
        }
        for (size_t i = 0; i < _handlerList.size(); ++i) {
            for (auto section : *_currentModuleSectionList) {
                _handlerList[i]->registerSection(section);
            }
        }
        _moduleSectionList.push_back(std::move(_currentModuleSectionList));
    }
}}
