#include <alibabacloud/oss/OssClient.h>
#include <string>
using namespace AlibabaCloud::OSS;
struct OSSInfo{
    std::string Bucket = "gogogxg";
    std::string Endpoint = "oss-cn-hangzhou.aliyuncs.com";
    std::string AccessKeyId = "LTAI5tASmKhd7bRmcKGgo4aP";
    std::string AccessKeySecret = "WMdsBLzjC9ei6u6h8z1hnxdXJ8rmjo"; 
};
int main(){
    //初始化网络连接
    InitializeSdk();
    OSSInfo info;
    //配置客户端属性（默认属性）
    ClientConfiguration conf;
    //创建一个客户端
    OssClient client(info.Endpoint,info.AccessKeyId,info.AccessKeySecret,conf);
    //std::string filepath = "Token.h";
    //auto outcome = client.PutObject(info.Bucket,"tmp/"+ filepath, filepath);//把本地的Token.h存入OSS的tmp/Token.h
    //if(!outcome.isSuccess()){
    //    fprintf(stderr,"PutObject fail, code = %s, message = %s, request id = %s\n", outcome.error().Code().c_str()
    //            ,outcome.error().Message().c_str()
    //            ,outcome.error().RequestId().c_str());
    //}
    time_t t = std::time(nullptr) + 1200;
    auto outcome = client.GeneratePresignedUrl(info.Bucket,"tmp/Token.h",t,Http::Get);
    if(!outcome.isSuccess()){
        fprintf(stderr,"PutObject fail, code = %s, message = %s, request id = %s\n", outcome.error().Code().c_str()
                ,outcome.error().Message().c_str()
                ,outcome.error().RequestId().c_str());
    }
    else{
        fprintf(stderr,"url = %s\n", outcome.result().c_str());
    }
    //释放连接
    ShutdownSdk();
}
