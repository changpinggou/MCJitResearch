//
//  ZQObjCInjector.h
//  ZQObjCInjector
//
//  Created by zuqingxie on 2019/5/15.
//  Copyright © 2019 zqx. All rights reserved.
//

#import <Foundation/Foundation.h>

//! Project version number for ZQObjCInjector.
FOUNDATION_EXPORT double ZQObjCInjectorVersionNumber;

//! Project version string for ZQObjCInjector.
FOUNDATION_EXPORT const unsigned char ZQObjCInjectorVersionString[];

// In this header, you should import all the public headers of your framework using statements like #import <ZQObjCInjector/PublicHeader.h>

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(int, ObjCinjectionState)
{
    ObjCinjectionState_success = 1,
    ObjCinjectionState_failed,
    ObjCinjectionState_wait, // 等待所有对象释放后再注入
};

typedef void (^ObjCinjectionStateBlock)(NSString *moduleName, ObjCinjectionState state);

@interface ZQObjCInjector : NSObject

@property (nonatomic, copy) ObjCinjectionStateBlock callback;

+ (BOOL)isAvaiable; //检测是否可用，只有处于debug模式下才可用

+ (id)sharedInstance;

- (void)startService:(NSString *)projectDir;
- (void)stopService;

- (void)notifyInstanceReleaseCompleted:(const char *)className; // 通知某个类的实例对象已经被全部销毁了，可以尝试注入某些依赖这个类的实例释放完成再注入的模块了
- (void)forceInject; // 强制注入未注入的模块

@end

NS_ASSUME_NONNULL_END
