//
//  ZQJITDemoViewController.m
//  ZQJITEngineDemo
//
//  Created by zuqingxie on 2019/4/16.
//  Copyright © 2019年 zqx. All rights reserved.
//

#import "ZQJITDemoViewController.h"

#define injected_code        0   // 这个宏用来开启测试代码的
#define injected_code_1      0   // 这个宏用来开启测试代码的

#if injected_code
@interface MyAddView : UIView
@property (nonatomic, assign) int countNum0;
@property (nonatomic, assign) double countNum;
@property (nonatomic, strong) UILabel *displayLabel;
@property (nonatomic, weak) NSTimer *updateTimer;
//@property (nonatomic, retain) UIView *disview;
//@property (nonatomic, strong) UIButton *btn;

@end

static NSString *helloworld()
{
    return [NSString stringWithFormat:@"oworld-%d", rand()];
}

static NSString *hello()
{
    return [NSString stringWithFormat:@"hello-%d", rand()];
}

@implementation MyAddView

- (id)initWithFrame:(CGRect)frame
{
    if (self = [super initWithFrame:frame]) {
        NSTimer *timer = [NSTimer scheduledTimerWithTimeInterval:1 repeats:YES block:^(NSTimer * _Nonnull timer) {
            [self onTimer];
        }];
        _updateTimer = timer;
        _countNum0 = 4;
        _countNum = -5;
        self.displayLabel = [UILabel new];
        _displayLabel.font = [UIFont systemFontOfSize:20];
        [self addSubview:_displayLabel];
        self.displayLabel.frame = self.bounds;
        _displayLabel.textColor = [UIColor redColor];
        //self.disview = _displayLabel;
        NSLog(@"MyAddView init %@---", self);
    }
    return self;
}

- (void)setCountNum:(double)countNum
{
    _countNum = countNum;
    self.displayLabel.text = [NSString stringWithFormat:@"%@：%0.1f", hello(), countNum];
    //self.disview.backgroundColor = [UIColor colorWithWhite:(rand() % 100) / 100.0 alpha:1];
}

- (void)onTimer
{
    self.countNum = self.countNum0 + _countNum + (arc4random() % 3) * 1.01;
    if (self.countNum > 20) {
        [_updateTimer invalidate];
        /*self.btn = [UIButton buttonWithType:UIButtonTypeSystem];
        [self.btn setTitle:@"close" forState:UIControlStateNormal];
        [self.btn addTarget:self action:@selector(ontap) forControlEvents:UIControlEventTouchUpInside];
        self.btn.frame = CGRectMake(0, 0, 50, 30);
        [self addSubview:_btn];
        NSLog(@"****************---------------- %@", _btn);
        [self.btn setTitleColor:[UIColor redColor] forState:UIControlStateNormal];*/
    }
}

- (void)ontap
{
    NSLog(@"-----------ontap-----------");
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(3 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        [self removeFromSuperview];
    });
}

- (void)dealloc
{
}

@end

#if injected_code_1
@interface MyAddView2 : MyAddView

@property (nonatomic, strong) UILabel *displayLabel2;

@end

@implementation MyAddView2

- (id)initWithFrame:(CGRect)frame
{
    NSLog(@"MyAddView2 class %p", self.class);
    if (self = [super initWithFrame:frame]) {

        self.displayLabel2 = [UILabel new];
        _displayLabel2.font = [UIFont systemFontOfSize:30];
        [self addSubview:_displayLabel2];
        self.displayLabel2.frame = CGRectMake(20, 20, 200, 100);
        _displayLabel2.textColor = [UIColor greenColor];
        NSLog(@"MyAddView2 init %@", self);
    }
    return self;
}

- (void)setCountNum:(double)countNum
{
    [super setCountNum:countNum];
    self.displayLabel2.text = [NSString stringWithFormat:@"计算：%f", countNum];
}

@end
#endif

#endif

#if injected_code
@interface ZQJITDemoViewController ()
@property (nonatomic, weak) NSString *weakMsgRef;
@end
#endif

@implementation ZQJITDemoViewController
#if injected_code
{
    NSString *_msg;
    MyAddView *_myAddView;
}
#endif

- (void)viewDidLoad
{
    [super viewDidLoad];
    self.view.backgroundColor = [UIColor whiteColor];
    UIButton *closeBtn = [UIButton buttonWithType:UIButtonTypeCustom];
    closeBtn.frame = CGRectMake((self.view.bounds.size.width - 100) / 2, 90, 100, 30);
    [closeBtn setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
    [closeBtn setTitle:@"关闭测试页" forState:UIControlStateNormal];
    [closeBtn addTarget:self action:@selector(onClose) forControlEvents:UIControlEventTouchUpInside];
    [self.view addSubview:closeBtn];
    
#if injected_code
    {
        _msg = [NSString stringWithFormat:@"hello,world!(from jit)%u", arc4random() % 100];
        _weakMsgRef = _msg;
        UILabel *label = [UILabel new];
        [self.view addSubview:label];
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.2 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            label.text = _weakMsgRef;
            [label sizeToFit];
            label.frame = CGRectMake((self.view.bounds.size.width - label.bounds.size.width) / 2, (self.view.bounds.size.height - label.bounds.size.height) / 2, label.bounds.size.width, label.bounds.size.height);
        });
#if injected_code_1
        Class myaddview2cls = NSClassFromString(@"MyAddView2");
        NSLog(@"line146:%p", myaddview2cls);
        _myAddView = [[MyAddView2 alloc] initWithFrame:CGRectMake((self.view.bounds.size.width - 100) / 2, (self.view.bounds.size.height - 50) / 2 + 100, 200, 50)];
        //NSLog(@"MyAddView2 cls = %p inst=%@", MyAddView2.class, _myAddView);
#else
        _myAddView = [[MyAddView alloc] initWithFrame:CGRectMake((self.view.bounds.size.width - 100) / 2, (self.view.bounds.size.height - 50) / 2 + 40, 200, 50)];
        //NSLog(@"MyAddView cls = %p inst=%@", MyAddView.class, _myAddView);
#endif
        [self.view addSubview:_myAddView];
    }
#endif
}

- (void)onClose
{
    [self dismissViewControllerAnimated:YES completion:nil];
}

@end
