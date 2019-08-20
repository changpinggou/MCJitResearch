//
//  ZQObjCInstCounter.m
//  ZQObjCInjector
//
//  Created by zuqingxie on 2019/5/26.
//  Copyright © 2019 zqx. All rights reserved.
//

#import "ZQObjCInstCounter.h"
#include <map>
#include <string>
#include <pthread.h>
#include <set>
#include <vector>
#import <objc/runtime.h>
#import <objc/message.h>
#import "ZQObjCInjector.h"

static std::map<std::string, uint32_t> g_ObjCInstCountMap;
static std::set<std::string> g_WaitingClassSet;
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;

static bool isClassInIgnoredList(void *obj)
{
    static dispatch_once_t onceToken;
    static const char *g_IgnoredClassName[1]; // 这里的类不能用iskindofclass判断，会crash
    static void *g_IgnoredClass[5];
    
    dispatch_once(&onceToken, ^{
        g_IgnoredClassName[0] = "_UIShareServiceActivityProxy_Share";
        
        g_IgnoredClass[0] = (__bridge void *)(NSString.class);
        g_IgnoredClass[1] = (__bridge void *)(NSArray.class);
        g_IgnoredClass[2] = (__bridge void *)(NSDictionary.class);
        g_IgnoredClass[3] = (__bridge void *)(NSSet.class);
        g_IgnoredClass[4] = (__bridge void *)(NSValue.class);
    });
    const char *className = class_getName([(__bridge id)obj class]);
    for (uint32_t i = 0; i < sizeof(g_IgnoredClassName) / sizeof(g_IgnoredClassName[0]); ++i) {
        if (0 == strcmp(g_IgnoredClassName[i], className)) {
            return true;
        }
    }
    for (uint32_t i = 0; i < sizeof(g_IgnoredClass) / sizeof(g_IgnoredClass[0]); ++i) {
        if ([(__bridge id)obj isKindOfClass:(__bridge Class)g_IgnoredClass[i]]) {
            return true;
        }
    }
    return false;
}

static void addClass(const char *className)
{
    if (!className) {
        return;
    }
    pthread_mutex_lock(&g_lock);
    std::map<std::string, uint32_t>::iterator iter = g_ObjCInstCountMap.find(className);
    if (iter != g_ObjCInstCountMap.end()) {
        iter->second++;
    } else {
        g_ObjCInstCountMap.insert(std::pair<std::string, uint32_t>(className, 1));
    }
    pthread_mutex_unlock(&g_lock);
}

static void removeClass(const char *className)
{
    if (!className) {
        return;
    }
    pthread_mutex_lock(&g_lock);
    std::map<std::string, uint32_t>::iterator iter = g_ObjCInstCountMap.find(className);
    if (iter != g_ObjCInstCountMap.end()) {
        iter->second--;
        if (0 == iter->second) {
            std::set<std::string>::iterator setIter = g_WaitingClassSet.find(iter->first);
            if (setIter != g_WaitingClassSet.end()) {
                g_WaitingClassSet.erase(setIter);
                std::string *releaseClass = new std::string(className);
                dispatch_async(dispatch_get_main_queue(), ^{
                    [[ZQObjCInjector sharedInstance] notifyInstanceReleaseCompleted:releaseClass->c_str()];
                    delete releaseClass;
                });
            }
            g_ObjCInstCountMap.erase(iter);
        }
    }
    pthread_mutex_unlock(&g_lock);
}

std::set<std::string> ZQObjCGetHasInstanceClasses(std::set<std::string> &classSet)
{
    std::set<std::string> hasInstanceClassSet;
    pthread_mutex_lock(&g_lock);
    for (std::set<std::string>::iterator it = classSet.begin(); it != classSet.end(); ++it) {
        if (g_ObjCInstCountMap.end() != g_ObjCInstCountMap.find(*it)) {
            g_WaitingClassSet.insert(*it);
            hasInstanceClassSet.insert(*it);
        }
    }
    pthread_mutex_unlock(&g_lock);
    return hasInstanceClassSet;
}

@implementation NSObject (ZQObjCInstLiftCycleTracker)

- (id)ZQObjC_init
{
    if (!isClassInIgnoredList((__bridge void *)self)) {
        addClass(class_getName(self.class));
    }
    return [self ZQObjC_init];
}

- (void)ZQObjC_dealloc
{
    if (!isClassInIgnoredList((__bridge void *)self)) {
        removeClass(class_getName(self.class));
    }
    [self ZQObjC_dealloc];
}

@end

static void ZQExchangeInstMethod(Class clz, SEL orgSel, SEL newSel)
{
    Method orgMethod = class_getInstanceMethod(clz, orgSel);
    Method newMethod = class_getInstanceMethod(clz, newSel);
    method_exchangeImplementations(orgMethod, newMethod);
}

@implementation ZQObjCInstCounter

+ (void)beginTrackObjCInstLifeCycle
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        ZQExchangeInstMethod(NSObject.class, @selector(init), @selector(ZQObjC_init));
        ZQExchangeInstMethod(NSObject.class, NSSelectorFromString(@"dealloc"), @selector(ZQObjC_dealloc));
    });
}

@end
