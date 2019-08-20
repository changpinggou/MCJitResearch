//
//  ZQJITCore.h
//  ZQJITEngine
//
//  Created by zuqingxie on 2019/4/11.
//  Copyright © 2019年 zqx. All rights reserved.
//

#ifndef ZQJITCore_h
#define ZQJITCore_h

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>
#include <llvm/IR/Mangler.h>
#include <algorithm>
#include <memory>
#include <string>
#include <map>

namespace zq { namespace jit {
    
    using namespace llvm;
    using namespace llvm::orc;
    
    class ZQBitCodeMouduleLoader
    {
    public:
        static std::unique_ptr<Module> loadModuleAtPath(const std::string &path);
    };
    
    class ZQJITCore
    {
    private:
        ExecutionSession _executionSession;
        std::shared_ptr<SymbolResolver> _symbolResolver;
        std::unique_ptr<TargetMachine> _targetMachine;
        const DataLayout _dataLayout;
        RTDyldObjectLinkingLayer _objectLayer;
        IRCompileLayer<decltype(_objectLayer), SimpleCompiler> _compileLayer;
        std::map<VModuleKey, VModuleKey> _moduleMap;
    public:
        ZQJITCore();
        ~ZQJITCore();
        VModuleKey addModule(std::unique_ptr<Module> module);
        void removeModule(VModuleKey key);
        JITSymbol findSymbol(const std::string &name);
    };
    
}}

#endif /* ZQJITCore_h */
