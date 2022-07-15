#ifndef __OSS_H__
#define __OSS_H__

#include <alibabacloud/oss/OssClient.h>
struct OSSInfo {
    std::string Bucket = "gogogxg";
    std::string Endpoint = "oss-cn-hangzhou.aliyuncs.com";
    std::string AccessKeyId = "LTAI5tASmKhd7bRmcKGgo4aP";
    std::string AccessKeySecret = "WMdsBLzjC9ei6u6h8z1hnxdXJ8rmjo";
};
enum {
    FS,
    OSS
};
struct Config {
    int storeType = OSS;
    int isAsyncTransferEnable = true;
};

#endif
