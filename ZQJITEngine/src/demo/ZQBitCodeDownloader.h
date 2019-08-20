//
//  ZQBitCodeDownloader.h
//  ZQJITEngine
//
//  Created by zuqingxie on 2019/4/9.
//  Copyright © 2019年 zqx. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface ZQBitCodeDownloader : NSObject

//@"http://10.64.69.105/PersonalLive/Source/App/PersonalLiveBiz/PersonalCenter/UI/PLMyCenterViewController.bc"
 
+ (void)downloadBitCodeWithURL:(NSString *)url complete:(void(^)(NSString *path))complete;

@end

NS_ASSUME_NONNULL_END
