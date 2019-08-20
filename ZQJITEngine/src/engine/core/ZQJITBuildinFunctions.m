//
//  ZQJITBuildinFunctions.m
//  ZQJITEngine
//
//  Created by zuqingxie on 2019/6/13.
//  Copyright © 2019 zqx. All rights reserved.
//

#import "ZQJITBuildinFunctions.h"
#import <UIKit/UIKit.h>

int __zqjit_isOSVersionAtLeast(int major, int minor, int subminor)
{
    static dispatch_once_t onceToken;
    static int g_OSVersionNum[3] = {0};
    dispatch_once(&onceToken, ^{
        NSArray<NSString *> *osVersionNum = [[[UIDevice currentDevice] systemVersion] componentsSeparatedByString:@"."];
        for (int i = 0; i < MIN((sizeof(g_OSVersionNum) / sizeof(g_OSVersionNum[0])), osVersionNum.count); ++i) {
            g_OSVersionNum[i] = [osVersionNum[i] intValue];
        }
    });
    if (major < g_OSVersionNum[0]) return 1;
    if (major > g_OSVersionNum[0]) return 0;
    if (minor < g_OSVersionNum[1]) return 1;
    if (minor > g_OSVersionNum[1]) return 0;
    return subminor <= g_OSVersionNum[2];
}
