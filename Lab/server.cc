#include <gxgfunc.h>
#include <workflow/HttpMessage.h>
#include <workflow/HttpUtil.h>
#include <workflow/WFFacilities.h>
#include <workflow/WFHttpServer.h>
#include <workflow/WFServer.h>
#include <workflow/WFTaskFactory.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

using Json = nlohmann::json;

struct SeriesContext {
    int fd;
    char* buf;
    size_t filesize;
    WFHttpTask* serverTask;
};

void httpCallback(WFHttpTask* serverTask) {
}

void IOCallback(WFFileIOTask* IOTask) {
    SeriesContext* context = static_cast<SeriesContext*>(series_of(IOTask)->get_context());
    auto resp2client = context->serverTask->get_resp();
    resp2client->add_header_pair("Content-Type", "text/html");
    resp2client->append_output_body(context->buf, context->filesize);
}

void process(WFHttpTask* serverTask) {
    protocol::HttpRequest* req = serverTask->get_req();
    protocol::HttpResponse* resp = serverTask->get_resp();
    std::string method = req->get_method();

    //分析url
    std::string url = req->get_request_uri();
    fprintf(stderr, "url = %s\n", url.c_str());
    if (url == "/file/mupload/init") {
        std::string username = "gxg";
        time_t now = time(0);
        tm* ltm = localtime(&now);
        std::string uploadID = username.append(":") + std::to_string(ltm->tm_hour) + ":" + std::to_string(ltm->tm_min);

        const void* body;
        size_t len;
        req->get_parsed_body(&body, &len);
        std::string wholeBody = (char*)body;

        Json fileJson = Json::parse(wholeBody);
        std::string filename = fileJson["filename"];
        std::string filehash = fileJson["filehash"];
        int filesize = fileJson["filesize"];

        int chunkCount = (filesize + 1) / 1000;

        //向客户端回复文件切片信息
        Json initJson;
        initJson["status"] = "OK";
        initJson["uploadID"] = uploadID;
        initJson["chunkcount"] = chunkCount;
        initJson["chunksize"] = 1000;
        initJson["filesize"] = filesize;
        initJson["filehash"] = filehash;
        resp->set_header_pair("Content-type", "text/plain");
        resp->append_output_body(initJson.dump());

        //创建redis任务
        WFRedisTask* redisTask1 = WFTaskFactory::create_redis_task("redis://127.0.0.1:6379", 0, nullptr);
        WFRedisTask* redisTask2 = WFTaskFactory::create_redis_task("redis://127.0.0.1:6379", 0, nullptr);
        WFRedisTask* redisTask3 = WFTaskFactory::create_redis_task("redis://127.0.0.1:6379", 0, nullptr);
        auto redisReq1 = redisTask1->get_req();
        auto redisReq2 = redisTask2->get_req();
        auto redisReq3 = redisTask3->get_req();
        redisReq1->set_request("HSET", {uploadID, "chunkcount", std::to_string(chunkCount)});
        redisReq2->set_request("HSET", {uploadID, "filehash", filehash});
        redisReq3->set_request("HSET", {uploadID, "filesize", std::to_string(filesize)});
        series_of(serverTask)->push_back(redisTask1);
        series_of(serverTask)->push_back(redisTask2);
        series_of(serverTask)->push_back(redisTask3);
        fprintf(stderr, "redis OK!\n");
    } else if (url == "/file/mupload/uppart") {
        
    } else if (url == "/file/mupload/complete") {
    } else {
        resp->set_header_pair("Content-type", "text/plain");
        resp->append_output_body("WrongUrl!");
    }
}

static WFFacilities::WaitGroup wait_group(1);

void sig_handler(int signo) {
    wait_group.done();
}

int main() {
    unsigned short port = 1234;
    signal(SIGINT, sig_handler);
    WFHttpServer server(process);
    if (server.start(port) == 0) {
        wait_group.wait();
        server.stop();
    } else {
        perror("Cannot start server");
        exit(1);
    }
    return 0;
}