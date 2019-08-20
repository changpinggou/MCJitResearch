//
//  ZQJITCore.cpp
//  ZQJITEngine
//
//  Created by zuqingxie on 2019/4/11.
//  Copyright © 2019年 zqx. All rights reserved.
//

#include "ZQJITCore.h"
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/Bitcode/BitcodeReader.h>
#include <llvm/Support/MemoryBuffer.h>
#include "ZQObjCSectionMemoryManager.h"
#include "ZQJITBuildinFunctions.h"

namespace zq { namespace jit {
    
    static llvm::LLVMContext g_llvmContext;
    
    std::unique_ptr<Module> ZQBitCodeMouduleLoader::loadModuleAtPath(const std::string &path)
    {
        auto memoryBuffer = MemoryBuffer::getFile(path);
        if (!memoryBuffer) {
            errs() << "ModuleLoader Can't open module: " << path << "\n";
            return nullptr;
        }
        errs() << "ModuleLoader open module: " << path << "\n";
        auto module = parseBitcodeFile(memoryBuffer->get()->getMemBufferRef(), g_llvmContext);
        if (!module) {
            errs() << "ModuleLoader Can't parse module: " << path << "\n" ;
            return nullptr;
        }
        return std::move(module.get());
    }
    
    ZQJITCore::ZQJITCore() :
    _symbolResolver(createLegacyLookupResolver(_executionSession,
                                               [this](const std::string &name) -> JITSymbol {
                                                   if (auto symbol = _compileLayer.findSymbol(name, false)) {
                                                       outs() << "symbol: " << name << ": " << symbol.getAddress().get() << "\n";
                                                       return symbol;
                                                   } else if (auto err = symbol.takeError()) {
                                                       return std::move(err);
                                                   }
                                                   if (auto symbolAddr = RTDyldMemoryManager::getSymbolAddressInProcess(name)) {
                                                       outs() << "symbol: " << name << ": " << symbolAddr << "\n";
                                                       return JITSymbol(symbolAddr, JITSymbolFlags::Exported);
                                                   }
                                                   if (name == "___isOSVersionAtLeast") {
                                                       // 支持@available(iOS 11.0, *)
                                                       uint64_t symbolAddr = (uint64_t)__zqjit_isOSVersionAtLeast;
                                                       outs() << "zqjit buildin symbol: " << name << ": " << symbolAddr << "\n";
                                                       return JITSymbol(symbolAddr, JITSymbolFlags::Exported);
                                                   }
                                                   errs() << "symbol: " << name << ": nullptr\n" ;
                                                   return nullptr;
                                               },
                                               [](Error err) {
                                                   cantFail(std::move(err), "lookupFlags failed");
                                               })),
    _targetMachine(EngineBuilder().selectTarget(llvm::Triple("arm64","apple","ios9.0.0"), "", "", SmallVector<std::string, 1>())),
    _objectLayer(_executionSession,
                 [this](VModuleKey) {
                     return RTDyldObjectLinkingLayer::Resources{std::make_shared<ZQObjCSectionMemoryManager>(), _symbolResolver};
                 }),
    _dataLayout(_targetMachine->createDataLayout()),
    _compileLayer(_objectLayer, SimpleCompiler(*_targetMachine))
    {
        
    }
    
    ZQJITCore::~ZQJITCore()
    {
        
    }
    
    VModuleKey ZQJITCore::addModule(std::unique_ptr<Module> module)
    {
        auto key = _executionSession.allocateVModule();
        cantFail(_compileLayer.addModule(key, std::move(module)));
        cantFail(_compileLayer.emitAndFinalize(key));
        _moduleMap[key] = key;
        return key;
    }
    
    void ZQJITCore::removeModule(VModuleKey key)
    {
        if (_moduleMap.find(key) != _moduleMap.end()) {
            _moduleMap.erase(key);
            cantFail(_compileLayer.removeModule(key));
        }
    }
    
    JITSymbol ZQJITCore::findSymbol(const std::string &name)
    {
        std::string mangledName;
        raw_string_ostream mangledNameStream(mangledName);
        Mangler::getNameWithPrefix(mangledNameStream, name, _dataLayout);
        return _compileLayer.findSymbol(mangledNameStream.str(), false);
    }
    
}}
