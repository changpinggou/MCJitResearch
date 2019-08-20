//
//  ZQObjCInstCounter.h
//  ZQObjCInjector
//
//  Created by zuqingxie on 2019/5/26.
//  Copyright © 2019 zqx. All rights reserved.
//

#import <Foundation/Foundation.h>
#include <string>
#include <set>

NS_ASSUME_NONNULL_BEGIN

@interface ZQObjCInstCounter : NSObject

+ (void)beginTrackObjCInstLifeCycle;

@end

std::set<std::string> ZQObjCGetHasInstanceClasses(std::set<std::string> &classSet); // 给定一个类集合，返回集合中类的实例个数>0的类集合

NS_ASSUME_NONNULL_END
