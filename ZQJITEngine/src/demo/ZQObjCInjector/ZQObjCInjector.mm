//
//  ZQObjCInjector.m
//  ZQObjCInjector
//
//  Created by zuqingxie on 2019/5/15.
//  Copyright © 2019 zqx. All rights reserved.
//

#import "ZQObjCInjector.h"
#import "ZQBitCodeReceiver.h"
#import <ZQJITEngine/ZQJITEngine.h>
#import "ZQObjCInstCounter.h"
#include <memory>
#include <string>
#include <vector>
#include <set>
#include <sys/types.h>
#include <sys/sysctl.h>

@interface ZQObjCInjector ()

@property (nonatomic, strong) ZQBitCodeReceiver *server;
@property (nonatomic, assign) std::vector<std::pair<ModuleContext, std::set<std::string>>> *waitingInjectModuleList; // <<模块指针,源文件全路径>,模块内等待释放实例的类>

@end

@implementation ZQObjCInjector

+ (BOOL)isAvaiable
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
    static ZQObjCInjector *g_instance;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        if ([self isAvaiable]) {
            g_instance = [self new];
            [ZQObjCInstCounter beginTrackObjCInstLifeCycle];
        }
    });
    return g_instance;
}

- (id)init
{
    if (self = [super init]) {
        _waitingInjectModuleList = new std::vector<std::pair<ModuleContext, std::set<std::string>>>();
    }
    return self;
}

- (void)dealloc
{
    for (auto module : *_waitingInjectModuleList) {
        [self releaseUninjectedModule:module.first.first];
    }
    delete _waitingInjectModuleList;
}

- (void)startService:(NSString *)projectDir
{
    if (_server) {
        return;
    }
    __weak typeof(self) weakSelf = self;
    _server = [[ZQBitCodeReceiver alloc] initWithProjectDir:projectDir complete:^(NSString * _Nonnull filePath) {
        dispatch_async(dispatch_get_main_queue(), ^{
            if ([[NSFileManager defaultManager] fileExistsAtPath:filePath] && weakSelf.server) {
                ModuleContext moduleCtx = [ZQJITEngine parseBitCodeFile:filePath];
                if (!moduleCtx.first) {
                    NSLog(@"parse bit code module %@ failed", filePath);
                    [weakSelf notifyInjectionState:ObjCinjectionState_failed module:filePath.lastPathComponent];
                    return;
                }
                NSString *sourceFileName = [NSString stringWithUTF8String:moduleCtx.second.c_str()];
                sourceFileName = [sourceFileName.lastPathComponent stringByDeletingPathExtension];
                
                std::set<std::string> objcClasses = [ZQJITEngine readObjCClasses:moduleCtx.first];
                std::set<std::string> hasInstanceClasses = ZQObjCGetHasInstanceClasses(objcClasses);
                bool hasInst = hasInstanceClasses.size() > 0;
                if (hasInst) {
                    // 先从等待队列中删除上一个相同的模块
                    for (auto moduleIter = weakSelf.waitingInjectModuleList->begin(); moduleIter != weakSelf.waitingInjectModuleList->end(); ++moduleIter) {
                        if (moduleIter->first.second == moduleCtx.second) {
                            weakSelf.waitingInjectModuleList->erase(moduleIter);
                            break;
                        }
                    }
                    weakSelf.waitingInjectModuleList->insert(weakSelf.waitingInjectModuleList->begin(), std::make_pair(moduleCtx, hasInstanceClasses));
                    NSLog(@"delay inject %@ wait for dealloc", sourceFileName);
                } else {
                    [[ZQJITEngine sharedInstance] loadBitCode:moduleCtx.first];
                    NSLog(@"immediate inject module %@ success", sourceFileName);
                }
                [weakSelf notifyInjectionState:hasInst ? ObjCinjectionState_wait : ObjCinjectionState_success module:sourceFileName];
            } else {
                NSLog(@"module %@ not found", filePath);
            }
        });
    }];
}

- (void)stopService
{
    _server = nil;
}

- (void)notifyInstanceReleaseCompleted:(const char *)className
{
    if (!className || 0 == strlen(className)) {
        return;
    }
    NSMutableString *injectedModuleNames = [NSMutableString new];
    for (auto moduleIter = _waitingInjectModuleList->begin(); moduleIter != _waitingInjectModuleList->end();) {
        for (auto classIter = moduleIter->second.begin(); classIter != moduleIter->second.end(); ++classIter) {
            if (*classIter == className) {
                moduleIter->second.erase(classIter);
                break;
            }
        }
        if (moduleIter->second.empty()) {
            // 这个模块所有依赖的类都已经销毁了，可以注入了
            [[ZQJITEngine sharedInstance] loadBitCode:moduleIter->first.first];
            NSLog(@"try inject module %s success", moduleIter->first.second.c_str());
            NSString *sourceFileName = [NSString stringWithUTF8String:moduleIter->first.second.c_str()];
            sourceFileName = [sourceFileName.lastPathComponent stringByDeletingPathExtension];
            [injectedModuleNames appendFormat:@"%@,", sourceFileName];
            moduleIter = _waitingInjectModuleList->erase(moduleIter);
        } else {
            ++moduleIter;
        }
    }
    if (injectedModuleNames.length > 0) {
        [self notifyInjectionState:ObjCinjectionState_success module:[injectedModuleNames substringToIndex:injectedModuleNames.length - 1]];
    } else {
        NSLog(@"not found module %s to be inject", className);
    }
}

- (void)forceInject
{
    NSMutableString *injectedModuleNames = [NSMutableString new];
    for (auto moduleIter = _waitingInjectModuleList->begin(); moduleIter != _waitingInjectModuleList->end();) {
        [[ZQJITEngine sharedInstance] loadBitCode:moduleIter->first.first];
        NSLog(@"force inject module %s", moduleIter->first.second.c_str());
        NSString *sourceFileName = [NSString stringWithUTF8String:moduleIter->first.second.c_str()];
        sourceFileName = [sourceFileName.lastPathComponent stringByDeletingPathExtension];
        [injectedModuleNames appendFormat:@"%@,", sourceFileName];
        moduleIter = _waitingInjectModuleList->erase(moduleIter);
    }
    if (injectedModuleNames.length > 0) {
        [self notifyInjectionState:ObjCinjectionState_success module:[injectedModuleNames substringToIndex:injectedModuleNames.length - 1]];
    }
}

- (void)releaseUninjectedModule:(llvm_Module)module
{
    [ZQJITEngine releaseUnloadedModule:module];
}

- (void)notifyInjectionState:(ObjCinjectionState)state module:(NSString *)moduleName
{
    if (self.callback) {
        self.callback(moduleName, state);
    }
}

@end
