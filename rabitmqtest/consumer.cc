#include <SimpleAmqpClient/SimpleAmqpClient.h>
#include <nlohmann/json.hpp>
#include "FileUtil.h"
#include "OSS.h"
#include "rabbitmq.h"

using namespace AlibabaCloud::OSS;
using Json = nlohmann::json;

void sigHandler(int num) {
    ShutdownSdk();
    fprintf(stderr, "consumer done\n");
    exit(EXIT_FAILURE);
}

int main() {
    signal(SIGINT, sigHandler);

    InitializeSdk();
    Config config;
    // 指定mq的一些信息
    RabbitMqInfo MqInfo;
    // 创建一条和mq的连接
    AmqpClient::Channel::ptr_t channel = AmqpClient::Channel::Create();
    channel->BasicConsume(MqInfo.TransQueueName, MqInfo.TransQueueName);
    // 创建一个信封用来获取消息
    AmqpClient::Envelope::ptr_t envelope;

    while (1) {
        // 从mq中提取消息
        // 获取消息最多等待五秒
        bool isNotTimeout = channel->BasicConsumeMessage(envelope, 5000);
        if (isNotTimeout == false) {  // 超时
            fprintf(stderr, "timeout\n");
            continue;
        }
        fprintf(stderr, "message = %s\n", envelope->Message()->Body().c_str());

        Json transInfo = nlohmann::json::parse(envelope->Message()->Body());
        std::string filepath = transInfo["filepath"];
        std::string filehash = transInfo["filehash"];
        std::string osspath = transInfo["osspath"];

        if (config.storeType == OSS) {
            OSSInfo info;
            ClientConfiguration conf;
            OssClient client(info.Endpoint, info.AccessKeyId, info.AccessKeySecret, conf);

            auto outcome = client.PutObject(info.Bucket, osspath, filepath);  //把本地的filepath存入OSS的osspath
            if (!outcome.isSuccess()) {
                fprintf(stderr, "PutObject fail, code = %s, message = %s, request id = %s\n", outcome.error().Code().c_str(), outcome.error().Message().c_str(), outcome.error().RequestId().c_str());
            } else {
                fprintf(stderr, "upload success!\n");
            }
        }
    }
    return EXIT_SUCCESS;
}