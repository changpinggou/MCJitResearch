//
//  ViewController.m
//  ObjCJitTest
//
//  Created by zuqingxie on 2019/3/31.
//  Copyright © 2019年 daliantu. All rights reserved.
//

#import "ViewController.h"
#import "ZQJITDemoViewController.h"
#import "ZQJITEngine.h"
#import "ZQObjCInjector.h"
#import "ZQBitCodeDownloader.h"

@interface ViewController ()

@property (nonatomic, strong) UILabel *tips;
@property (nonatomic, strong) UIButton *exec;
@property (nonatomic, assign) BOOL running;

@end

@implementation ViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    self.tips = [[UILabel alloc] initWithFrame:CGRectMake(0, 48, self.view.bounds.size.width, 22)];
    self.tips.font = [UIFont systemFontOfSize:18];
    self.tips.textAlignment = NSTextAlignmentCenter;
    self.tips.text = @"空闲中...";
    [self.view addSubview:self.tips];
    
    self.exec = [UIButton buttonWithType:UIButtonTypeCustom];
    self.exec.frame = CGRectMake(20, 90, 100, 30);
    [self.exec setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
    [self.exec setTitle:@"注入代码" forState:UIControlStateNormal];
    [self.exec addTarget:self action:@selector(onClick) forControlEvents:UIControlEventTouchUpInside];
    [self.view addSubview:self.exec];
    
    UIButton *openTestPageBtn = [UIButton buttonWithType:UIButtonTypeCustom];
    openTestPageBtn.frame = CGRectMake(180, 90, 100, 30);
    [openTestPageBtn setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
    [openTestPageBtn setTitle:@"进入测试页" forState:UIControlStateNormal];
    [openTestPageBtn addTarget:self action:@selector(onOpenTestPage) forControlEvents:UIControlEventTouchUpInside];
    [self.view addSubview:openTestPageBtn];
    [[ZQObjCInjector sharedInstance] startService:__XCODE_PROJECT_DIR__];
}

- (void)setRunning:(BOOL)running
{
    _running = running;
    [self.exec setTitle:running ? @"注入中..." : @"注入代码" forState:UIControlStateNormal];
}

- (void)onClick
{
    if (self.running) {
        return;
    }

    self.tips.text = @"下载文件中...";
    [ZQBitCodeDownloader downloadBitCodeWithURL:@"" complete:nil];
    NSURLSession *session = [NSURLSession sharedSession];
    NSURL *bitCodeURL = [NSURL URLWithString:[@"http://10.64.69.54/zuqingxie/llvm-jit项目/project/ZQJITEngine/ZQJITEngine/src/demo/ZQJITDemoViewController.bc" stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]];
    NSURLSessionDownloadTask *downloadTask = [session downloadTaskWithURL:bitCodeURL completionHandler:^(NSURL * _Nullable location, NSURLResponse * _Nullable response, NSError * _Nullable error) {
        BOOL downloadSuc = (!error && location);
        NSLog(@"下载bitcode文件%@", downloadSuc ? @"成功" : @"失败");
        if (downloadSuc) {
            NSString *docPath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) lastObject];
            NSString *bitCodePath = [NSString stringWithFormat:@"%@/%@", docPath, response.suggestedFilename];
            NSFileManager *fmgr = [NSFileManager defaultManager];
            [fmgr removeItemAtPath:bitCodePath error:nil];
            BOOL removeBitCodeSuc = [fmgr moveItemAtPath:location.path toPath:bitCodePath error:nil];
            dispatch_async(dispatch_get_main_queue(), ^{
                if (removeBitCodeSuc) {
                    [[ZQJITEngine sharedInstance] loadBitCodeAtPath:bitCodePath];
                    self.tips.text = [NSString stringWithFormat:@"注入完成"];
                } else {
                    NSLog(@"移动bitcode文件到%@失败", bitCodePath);
                    self.tips.text = [NSString stringWithFormat:@"移动文件失败"];
                }
            });
        } else {
            dispatch_async(dispatch_get_main_queue(), ^{
                self.tips.text = [NSString stringWithFormat:@"下载文件失败"];
                self.running = NO;
            });
        }
    }];
    [downloadTask resume];
}

- (void)onOpenTestPage
{
    [self presentViewController:[ZQJITDemoViewController new] animated:YES completion:nil];
}

@end
