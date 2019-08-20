#import <UIKit/UIKit.h>
#include <stdio.h>

@interface FirstClass : NSObject

- (int)hello;

@end

@implementation FirstClass

- (int)hello 
{
   return 666;
}

- (void)dealloc
{

}
@end

@interface ZQWebViewController : UIViewController

@property (nonatomic, strong) UIWebView *webview;

@end

@implementation ZQWebViewController

- (void)viewDidLoad 
{
   [super viewDidLoad];
   self.webview = [[UIWebView alloc] initWithFrame:self.view.bounds];
   [self.webview loadRequest:[NSURLRequest requestWithURL:[NSURL URLWithString:@"https://www.qq.com"]]];
   [self.view addSubview:self.webview];
}

@end

int run() 
{
    NSString *tips = [NSString stringWithFormat:@"我是objc-jit:9999"];
    UIAlertView *WXinstall=[[UIAlertView alloc]initWithTitle:tips message:tips delegate:nil cancelButtonTitle:tips otherButtonTitles:tips, nil];//一般在if判断中加入
    [WXinstall show];

   //  UIViewController *vc = [UIApplication sharedApplication].keyWindow.rootViewController;
   //  [vc presentViewController:[ZQWebViewController new] animated:YES completion:nil];
    return 7788;
}

