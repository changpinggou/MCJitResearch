//
//  ZQBitCodeDownloader.m
//  ZQJITEngine
//
//  Created by zuqingxie on 2019/4/9.
//  Copyright © 2019年 zqx. All rights reserved.
//

#import "ZQBitCodeDownloader.h"

@implementation ZQBitCodeDownloader

- (void)heheda
{
    NSLog(@"--------------0000000--------------");
    [self.class say2];
}

+ (void)say2
{
    NSLog(@"say2-------8888--------");
}

+ (void)downloadBitCodeWithURL:(NSString *)url complete:(void(^)(NSString *path))complete
{
    [[self new] heheda];
    static int runTimes = 0;
    runTimes++;
    if (runTimes < 2) {
        if (complete) {
            complete(nil);
        }
        return;
    }

    if (url.length == 0) {
        if (complete) {
            complete(nil);
        }
        return;
    }
    NSURLSession *session = [NSURLSession sharedSession];
    NSURL *bitCodeURL = [NSURL URLWithString:[url stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]];
    NSURLSessionDownloadTask *downloadTask = [session downloadTaskWithURL:bitCodeURL completionHandler:^(NSURL * _Nullable location, NSURLResponse * _Nullable response, NSError * _Nullable error) {
        BOOL downloadSuc = (!error && location) && (((NSHTTPURLResponse *)response).statusCode == 200);
        NSLog(@"下载bitcode文件%@", downloadSuc ? @"成功" : @"失败");
        if (downloadSuc) {
            NSString *docPath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) lastObject];
            NSString *bitCodePath = [NSString stringWithFormat:@"%@/%@", docPath, response.suggestedFilename];
            NSFileManager *fmgr = [NSFileManager defaultManager];
            [fmgr removeItemAtPath:bitCodePath error:nil];
            BOOL removeBitCodeSuc = [fmgr moveItemAtPath:location.path toPath:bitCodePath error:nil];
            dispatch_async(dispatch_get_main_queue(), ^{
                if (removeBitCodeSuc) {
                    if (complete) {
                        complete(bitCodePath);
                    }
                } else {
                    NSLog(@"移动bitcode文件到%@失败", bitCodePath);
                    if (complete) {
                        complete(nil);
                    }
                }
            });
        } else {
            dispatch_async(dispatch_get_main_queue(), ^{
                if (complete) {
                    complete(nil);
                }
            });
        }
    }];
    [downloadTask resume];
}

@end
