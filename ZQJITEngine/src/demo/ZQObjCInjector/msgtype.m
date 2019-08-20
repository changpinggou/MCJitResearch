//
//  msgtype.m
//  ZQObjCInjector
//
//  Created by zuqingxie on 2019/5/20.
//  Copyright © 2019 zqx. All rights reserved.
//

#import "msgtype.h"

dispatch_data_t EncodeStringData(NSString *message)
{
    // Use a custom struct
    const char *utf8text = [message cStringUsingEncoding:NSUTF8StringEncoding];
    size_t length = strlen(utf8text);
    ZQTextFrame *textFrame = CFAllocatorAllocate(nil, sizeof(ZQTextFrame) + length, 0);
    memcpy(textFrame->utf8text, utf8text, length); // Copy bytes to utf8text array
    textFrame->length = htonl(length); // Convert integer to network byte order
    
    // Wrap the textFrame in a dispatch data object
    return dispatch_data_create((const void*)textFrame, sizeof(ZQTextFrame)+length, nil, ^{
        CFAllocatorDeallocate(nil, textFrame);
    });
}

NSString *DecodeStringData(dispatch_data_t dispatchData)
{
    NSData *data = (NSData *)dispatchData;
    return [[NSString alloc] initWithBytes:data.bytes + 4 length:data.length - 4 encoding:NSUTF8StringEncoding];
}

dispatch_data_t EncodeFileData(NSString *filePath)
{
    NSData *fileData = [NSData dataWithContentsOfFile:filePath];
    if (fileData) {
        return dispatch_data_create(fileData.bytes, fileData.length, nil, nil);
    }
    return nil;
}

@implementation ZQChannelContext

- (NSString *)description
{
    return [NSString stringWithFormat:@"[deviceID:%@ port:%d]", _deviceID, _port];
}

@end
