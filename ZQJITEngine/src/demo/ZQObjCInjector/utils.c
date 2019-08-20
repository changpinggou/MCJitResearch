//
//  utils.c
//  ZQObjCInjector
//
//  Created by zuqingxie on 2019/5/20.
//  Copyright © 2019 zqx. All rights reserved.
//

#include "utils.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <memory.h>
#include <unistd.h>

int is_port_aviable(short port)
{
    int fd = -1;
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    
    fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd < 0) {
        printf("socket() error:%s\n", strerror(errno));
        return 0;
    }
    if (bind(fd, (struct sockaddr *)&sin, sizeof(sin)) != 0) {
        printf("bind() error:%s\n", strerror(errno));
        close(fd);
        return 0;
    }
    socklen_t len;
    if(getsockname(fd, (struct sockaddr *)&sin, &len) != 0) {
        printf("getsockname() error:%s\n", strerror(errno));
        close(fd);
        return 0;
    }
    
    if(fd != -1) {
        close(fd);
    }
    
    return 1;
}

int get_server_port(void)
{
    for (int port = SERVER_PORT_0; port <= SERVER_PORT_2; ++port) {
        if (is_port_aviable(port)) {
            return port;
        }
    }
    return -1;
}
