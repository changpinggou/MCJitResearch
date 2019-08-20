//
//  ZQJITEngine.h
//  ZQJITEngine
//
//  Created by zuqingxie on 2019/4/11.
//  Copyright © 2019年 zqx. All rights reserved.
//

#import <Foundation/Foundation.h>
#include <memory>
#include <string>
#include <set>

NS_ASSUME_NONNULL_BEGIN

typedef uint8_t * llvm_Module;
typedef std::pair<llvm_Module, std::string> ModuleContext; // <模块指针,源文件全路径>

@interface ZQJITEngine : NSObject

// 判断JIT是否可用(仅仅在XCode直连手机调试模式下才能运行JIT，其他情况下JIT一律不可用)
+ (BOOL)isAviable;

+ (id)sharedInstance;

+ (ModuleContext)parseBitCodeFile:(NSString *)path; // 解析模块，注意该模块指针不能私自保存，如果最后未加载，则需要调用releaseUnloadedModule来释放

+ (void)releaseUnloadedModule:(llvm_Module)module; // 释放未加载的模块，用于释放从parseBitCodeFile创建的模块，如果module已经被loadBitCode加载，则无需释放

+ (std::set<std::string>)readObjCClasses:(llvm_Module)module; // 读取模块内实现的Objctive-C类

- (void)loadBitCode:(llvm_Module)module; // 加载paeseBitCode解析出来的模块

- (void)loadBitCodeAtPath:(NSString *)path; //快捷方法，内部调用parseBitCodeFile和loadBitCode

- (void)runFunction:(NSString *)funcName; // 执行一个函数，函数为返回值和参数均为void

@end

NS_ASSUME_NONNULL_END
