//
//  ZQJITEngine.m
//  ZQJITEngine
//
//  Created by zuqingxie on 2019/4/11.
//  Copyright © 2019年 zqx. All rights reserved.
//

#import "ZQJITEngine.h"
#include "ZQJITCore.h"
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>
#include <sys/types.h>
#include <sys/sysctl.h>

using namespace llvm;
using namespace llvm::orc;
using namespace zq::jit;

using ZQJITItem = std::pair<std::shared_ptr<ZQJITCore>, std::vector<std::string>>; // 一个ZQJITCore下的模块必须不相同（同一个源码多次重新编译后的模块即为相同模块），如果有相同的模块加载进来，就要新建一个ZQJITCore

@implementation ZQJITEngine
{
    std::vector<ZQJITItem> _jitItems;
}

+ (BOOL)isAviable;
{
    size_t size = sizeof(struct kinfo_proc);
    struct kinfo_proc info;
    int ret, name[4];
    memset(&info, 0, sizeof(struct kinfo_proc));
    name[0] = CTL_KERN;
    name[1] = KERN_PROC;
    name[2] = KERN_PROC_PID;
    name[3] = getpid();
    if((ret = (sysctl(name, 4, &info, &size, NULL, 0)))){
        return NO;
    }
    return info.kp_proc.p_flag & P_TRACED;
}

+ (id)sharedInstance
{
    static dispatch_once_t token;
    static ZQJITEngine *s_engine = nil;
    dispatch_once(&token, ^{
        if ([self isAviable]) {
            llvm::InitializeNativeTarget();
            llvm::InitializeNativeTargetAsmPrinter();
            llvm::InitializeNativeTargetAsmParser();
            llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
            s_engine = [self new];
        }
    });
    return s_engine;
}

+ (ModuleContext)parseBitCodeFile:(NSString *)path
{
    if (path.length > 0) {
        auto module = ZQBitCodeMouduleLoader::loadModuleAtPath(path.UTF8String);
        if (module) {
            std::string sourceFileName = module->getSourceFileName();
            return std::make_pair((llvm_Module)module.release(), sourceFileName);
        }
    }
    return std::make_pair(nullptr, std::string());
}

+ (void)releaseUnloadedModule:(llvm_Module)module
{
    if (!module) {
        return;
    }
    llvm::Module *modulePtr = (llvm::Module *)module;
    delete modulePtr;
}

+ (std::set<std::string>)readObjCClasses:(llvm_Module)module
{
    std::set<std::string> objcClassList;
    if (module) {
        llvm::Module *modulePtr = (llvm::Module *)module;
        for (auto it = modulePtr->global_begin(); it != modulePtr->global_end(); ++it) {
            std::string globalValueName = it->getName();
            size_t objcClassPrefixLength = sizeof("OBJC_CLASS_$_") - 1;
            if (globalValueName.length() > objcClassPrefixLength && 0 == globalValueName.find("OBJC_CLASS_$_") && it->hasSection()) {
                std::string className = globalValueName.substr(objcClassPrefixLength, globalValueName.length() - objcClassPrefixLength);
                objcClassList.insert(className);
            }
        }
    }
    return objcClassList;
}

- (id)init
{
    if (self = [super init]) {
        
    }
    return self;
}

- (void)dealloc
{
    
}

- (void)loadBitCode:(llvm_Module)module
{
    if (!module) {
        return;
    }
    llvm::Module *modulePtr = (llvm::Module *)module;
    std::string sourceFileName = modulePtr->getSourceFileName();
    int64_t jitItemIndex = -1;
    for (auto i = 0; i < _jitItems.size(); ++i) {
        ZQJITItem jitItem = _jitItems[i];
        std::vector<std::string> sourceFileNameList = jitItem.second;
        bool exsited = false;
        for (auto fileName : sourceFileNameList) {
            if (!fileName.compare(sourceFileName)) {
                exsited = true;
                break;
            }
        }
        if (!exsited) {
            jitItemIndex = i;
            break;
        }
    }
    if (-1 == jitItemIndex) {
        std::pair<std::shared_ptr<ZQJITCore>, std::vector<std::string>> jitItem = std::make_pair(std::shared_ptr<ZQJITCore>(new ZQJITCore), std::vector<std::string>());
        _jitItems.push_back(jitItem);
        jitItemIndex = _jitItems.size() - 1;
        errs() << "new jit core " << jitItemIndex << "\n";
    }
    ZQJITItem &jitItem = _jitItems[jitItemIndex];
    jitItem.first->addModule(std::unique_ptr<llvm::Module>(modulePtr));
    jitItem.second.push_back(sourceFileName);
    errs() << "find jit core index " << jitItemIndex << "\n";
}

- (void)loadBitCodeAtPath:(NSString *)path
{
    std::pair<llvm_Module, std::string> moduleCtx = [self.class parseBitCodeFile:path];
    [self loadBitCode:moduleCtx.first];
}

- (void)runFunction:(NSString *)funcName
{
    for (auto i = _jitItems.size(); i > 0; --i) {
        ZQJITItem jitItem = _jitItems[i - 1];
        JITSymbol symbol = jitItem.first->findSymbol(funcName.UTF8String);
        void (*func)() = (void (*)())cantFail(symbol.getAddress());
        if (func) {
            func();
            errs() << "run function: " << funcName.UTF8String << "\n";
            break;
        }
    }
}

@end
