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

static WFFacilities::WaitGroup wait_group(1);

void sig_handler(int signo) {
    wait_group.done();
}

void process(WFHttpTask* serverTask) {
    // 解析url，分派任务
    auto req = serverTask->get_req();
    auto resp = serverTask->get_resp();
    std::string uri = req->get_request_uri();
    std::string path = uri.substr(0, uri.find("?"));
    std::string query = uri.substr(uri.find("?") + 1);
    std::string method = req->get_method();

    if (method == "POST" && path == "/file/mupload/init") {
        //初始化
        // 1 读取请求报文，获取请求报文体
        const void* body;
        size_t len;
        req->get_parsed_body(&body, &len);
        std::string wholeBody = (char*)body;

        // 2 将报文体解析成json对象
        Json fileInfo = Json::parse(wholeBody);
        std::string filename = fileInfo["filename"];
        std::string filehash = fileInfo["filehash"];
        int filesize = fileInfo["filesize"];

        // 3 初始化分块信息 uploadID
        std::string username = "gxg";
        time_t now = time(0);
        tm* ltm = localtime(&now);
        std::string uploadID = username.append(":") + std::to_string(ltm->tm_hour) + ":" + std::to_string(ltm->tm_min);
        //生成分块信息
        int chunkcount;
        int chunksize = 5 * 1024 * 1024;
        chunkcount = filesize / chunksize + (filesize % chunksize != 0);
        Json uppartInfo;
        uppartInfo["status"] = "OK";
        uppartInfo["uploadID"] = uploadID;
        uppartInfo["chunkcount"] = chunkcount;
        uppartInfo["chunksize"] = chunksize;
        uppartInfo["filesize"] = filesize;
        uppartInfo["filehash"] = filehash;

        // 5 将一些信息写入缓存
        //创建redis任务
        std::vector<std::vector<std::string>> argsVec = {
            {"MP_" + uploadID, "chunkcount", std::to_string(chunkcount)},
            {"MP_" + uploadID, "filehash", filehash},
            {"MP_" + uploadID, "filesize", std::to_string(filesize)}};
        for (int i = 0; i < 3; ++i) {
            WFRedisTask* redisTask = WFTaskFactory::create_redis_task("redis://127.0.0.1:6379", 0, nullptr);
            redisTask->get_req()->set_request("HSET", argsVec[i]);
            redisTask->start();
        }
        resp->set_header_pair("Content-type", "text/plain");
        resp->append_output_body(uppartInfo.dump());

        // 4 生成对客户端的响应
    } else if (method == "POST" && path == "/file/mupload/uppart") {
        //上传单个分块
        // 1 解析用户请求 提取出uploadID和chkidx
        std::string uploadIDKV = query.substr(0, query.find("&"));
        std::string chkidxKV = query.substr(query.find("&") + 1);
        std::string uploadID = uploadIDKV.substr(uploadIDKV.find("=") + 1);
        std::string chkidx = chkidxKV.substr(chkidxKV.find("=") + 1);

        // 2 获取文件的文件名，创建目录，写入分块
        auto redisTaskHGET = WFTaskFactory::create_redis_task("redis://127.0.0.1:6379", 2, [chkidx, req](WFRedisTask* redisTask) {
            protocol::RedisRequest* redisReq = redisTask->get_req();
            protocol::RedisResponse* redisResp = redisTask->get_resp();
            protocol::RedisValue value;  // value对象专门用来存储redis任务的结果
            redisResp->get_result(value);
            // 创建空文件
            std::string filehash = value.string_value();
            mkdir(filehash.c_str(), 0777);
            std::string filepath = filehash + "/" + chkidx;
            int fd = open(filepath.c_str(), O_RDWR | O_CREAT, 0666);
            // 将文件内容进行写入
            const void* body;
            size_t size;
            req->get_parsed_body(&body, &size);
            write(fd, body, size);
            close(fd);
        });
        redisTaskHGET->get_req()->set_request("HGET", {uploadID, "filehash"});
        series_of(serverTask)->push_back(redisTaskHGET);

        // 3 写入分块完成之后，将上传的进度存入缓存
        auto redisTaskHSET = WFTaskFactory::create_redis_task("redis://127.0.0.1:6379", 2, nullptr);
        redisTaskHSET->get_req()->set_request("HSET", {uploadID, "chkidx_" + chkidx, "1"});
        series_of(serverTask)->push_back(redisTaskHSET);

        // 4 回复响应
        resp->append_output_body("OK");
    } else if (method == "GET" && path == "/file/mupload/complete") {
        //合并分块
        // 1 解析用户请求 提取出uploadID
        std::string uploadID = query.substr(query.find("=") + 1);
        auto redisTask = WFTaskFactory::create_redis_task("redis://127.0.0.1:6379", 2, [resp](WFRedisTask* redisTask) {
            protocol::RedisRequest* redisReq = redisTask->get_req();
            protocol::RedisResponse* redisResp = redisTask->get_resp();
            protocol::RedisValue value;  // value对象专门用来存储redis任务的结果
            redisResp->get_result(value);
            // 3 chunkcount
            int chunkcount;
            int chunknow = 0;
            for (int i = 0; i < value.arr_size(); i += 2) {
                std::string key = value.arr_at(i).string_value();
                std::string val = value.arr_at(i + 1).string_value();
                if (key == "chunkcount") {
                    chunkcount = std::stoi(val);
                } else if (key.substr(0, 7) == "chkidx_") {
                    ++chunknow;
                }
            }
            // 4 比较大小
            if (chunkcount == chunknow) {
                resp->append_output_body("SUCCESS");
            } else {
                resp->append_output_body("FAIL");
            }
        });
        redisTask->get_req()->set_request("HGETALL", {uploadID});
        series_of(serverTask)->push_back(redisTask);
        // 2 根据upload查询进度 HGETALL uploadID
    } else {
        resp->set_header_pair("Content-type", "text/plain");
        resp->append_output_body("WrongUrl!");
    }
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