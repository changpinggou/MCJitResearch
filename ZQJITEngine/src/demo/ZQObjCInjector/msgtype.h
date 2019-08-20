//
//  msgtype.h
//  ZQObjCInjector
//
//  Created by zuqingxie on 2019/5/20.
//  Copyright © 2019 zqx. All rights reserved.
//

#import <Foundation/Foundation.h>

enum {
    ZQFrameTypeWorkProjectSyc = 100,
    ZQFrameTypeWorkProjectAck = 101,
    ZQFrameTypeBitCodeFile    = 102,
};

typedef struct _PTExampleTextFrame {
    uint32_t length;
    uint8_t utf8text[0];
} ZQTextFrame;

#ifdef __cplusplus
extern "C" {
#endif

dispatch_data_t EncodeStringData(NSString *message);
NSString *DecodeStringData(dispatch_data_t dispatchData);

dispatch_data_t EncodeFileData(NSString *filePath);

#ifdef __cplusplus
}
#endif

@interface ZQChannelContext : NSObject

@property (nonatomic, strong) NSNumber *deviceID;
@property (nonatomic, assign) int port;

@end
