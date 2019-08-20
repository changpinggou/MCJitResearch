//
//  ZQBitCodeReceiver.m
//  ZQObjCInjector
//
//  Created by zuqingxie on 2019/5/15.
//  Copyright © 2019 zqx. All rights reserved.
//

#import "ZQBitCodeReceiver.h"
#import <Peertalk/Peertalk.h>
#include "utils.h"
#import "msgtype.h"

@interface ZQBitCodeReceiver () <PTChannelDelegate>

@property (nonatomic, copy) NSString *projectDir;
@property (nonatomic, strong) PTChannel *serverChannel;
@property (nonatomic, strong) PTChannel *clientChannel;
@property (nonatomic, strong) NSMutableArray<PTChannel *> *candidateClientArray;
@property (nonatomic, strong) dispatch_queue_t queue;
@property (nonatomic, copy) void (^complete)(NSString *filePath);

@end

@implementation ZQBitCodeReceiver

- (id)initWithProjectDir:(NSString *)projectDir complete:(void(^)(NSString *filePath))complete
{
    if (self = [super init]) {
        self.projectDir = projectDir;
        self.complete = complete;
        self.candidateClientArray = [NSMutableArray new];
        [self startListener];
    }
    return self;
}

- (void)dealloc
{
    [self.clientChannel close];
    [self.serverChannel close];
    [self.candidateClientArray enumerateObjectsUsingBlock:^(PTChannel * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        [obj close];
    }];
}

- (void)startListener
{
    self.queue = dispatch_queue_create("com.zq.bitcodereceiver", NULL);
    PTChannel *channel = [[PTChannel alloc] initWithProtocol:[PTProtocol sharedProtocolForQueue:self.queue] delegate:self];
    __weak typeof(self) weakSelf = self;
    int port = get_server_port();
    [channel listenOnPort:port IPv4Address:INADDR_LOOPBACK callback:^(NSError *error) {
        if (!error) {
            weakSelf.serverChannel = channel;
        }
        NSLog(@"listen on port %d %@", port, error ? error : @"success");
    }];
}

- (void)sendWorkProjectMsg:(PTChannel *)otherChannel
{
    [otherChannel sendFrameOfType:ZQFrameTypeWorkProjectSyc tag:0 withPayload:EncodeStringData(self.projectDir) callback:^(NSError *error) {
        NSLog(@"send project directory syc %@", error ? error : @"success");
    }];
}

#pragma mark PTChannelDelegate
// Invoked when a new frame has arrived on a channel.
- (void)ioFrameChannel:(PTChannel*)channel didReceiveFrameOfType:(uint32_t)type tag:(uint32_t)tag payload:(PTData*)payload
{
    if (type == ZQFrameTypeWorkProjectAck) {
        NSString *projectDir = DecodeStringData(payload.dispatchData);
        if ([projectDir isEqualToString:self.projectDir] && [self.candidateClientArray containsObject:channel]) {
            NSLog(@"connected with project %@ old channel %p new channel %p", projectDir, _clientChannel, channel);
            if (self.clientChannel && channel != self.clientChannel) {
                [self.clientChannel close];
            }
            self.clientChannel = channel;
        } else {
            [channel close];
        }
        [self.candidateClientArray removeObject:channel];
    } else if (type == ZQFrameTypeBitCodeFile) {
        if (self.clientChannel != channel || !self.complete) {
            return;
        }
        NSString *filePath = [NSString stringWithFormat:@"%@/bitcode_%llu.bc", NSTemporaryDirectory(), (uint64_t)(CFAbsoluteTimeGetCurrent() * 1000)];
        [(NSData *)payload.dispatchData writeToFile:filePath atomically:YES];
        self.complete(filePath);
    } else {
        [channel close];
    }
}

// Invoked when the channel closed. If it closed because of an error, *error* is
// a non-nil NSError object.
- (void)ioFrameChannel:(PTChannel*)channel didEndWithError:(NSError*)error
{
    NSLog(@"close connection %p %@", channel, error);
    if (channel == self.clientChannel) {
        self.clientChannel = nil;
    }
    [self.candidateClientArray removeObject:channel];
}

// For listening channels, this method is invoked when a new connection has been
// accepted.
- (void)ioFrameChannel:(PTChannel*)channel didAcceptConnection:(PTChannel*)otherChannel fromAddress:(PTAddress*)address
{
    NSLog(@"accept connection %@:%ld channel %p",address.name, address.port, channel);
    [self.candidateClientArray addObject:otherChannel];
    dispatch_async(self.queue, ^{
        [self sendWorkProjectMsg:otherChannel];
    });
}

@end
