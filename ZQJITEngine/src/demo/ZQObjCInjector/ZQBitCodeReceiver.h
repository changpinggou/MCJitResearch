//
//  ZQBitCodeReceiver.h
//  ZQObjCInjector
//
//  Created by zuqingxie on 2019/5/15.
//  Copyright © 2019 zqx. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface ZQBitCodeReceiver : NSObject

- (id)initWithProjectDir:(NSString *)projectDir complete:(void(^)(NSString *filePath))complete;

@end

NS_ASSUME_NONNULL_END
