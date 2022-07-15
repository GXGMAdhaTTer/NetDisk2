#include "FileCallbackLib.h"
#include <workflow/MySQLMessage.h>
#include <workflow/MySQLResult.h>
#include <workflow/WFFacilities.h>
#include <wfrest/json.hpp>
#include "FileUtil.h"

using namespace AlibabaCloud::OSS;
using Json = nlohmann::json;

FileCallbackLib::FileCallbackLib(Config& config)
    : _config(config) {}

void FileCallbackLib::fileUploadGet(const wfrest::HttpReq* req, wfrest::HttpResp* resp) {
    resp->File("../static/view/index.html");
}

void FileCallbackLib::fileUploadPost(const wfrest::HttpReq* req, wfrest::HttpResp* resp) {
    // 从URL中提取用户名的信息
    auto userInfo = req->query_list();
    std::string username = userInfo["username"];
    //  读取文件内容 解析form-data类型的请求报文
    using Form = std::map<std::string, std::pair<std::string, std::string>>;
    Form& form = req->form();
    std::pair<std::string, std::string> fileInfo = form["file"];
    // fileInfo.first fileInfo.second
    std::string filepath = "../tmp/" + fileInfo.first;
    int fd = open(filepath.c_str(), O_RDWR | O_CREAT, 0666);
    if (fd < 0) {
        resp->set_status_code("500");
        return;
    }
    // try {} catch ()
    // 方案 1 write
    int ret = write(fd, fileInfo.second.c_str(), fileInfo.second.size());

    close(fd);

    /* 方案 2 pwrite的任务
    auto pwriteTask = WFTaskFactory::create_pwrite_task(fd,fileInfo.second.c_str(),fileInfo.second.size(),0,callback);
    //把上传文件之后的逻辑写到callback当中
    series->push_back(pwriteTask)
    */

    std::string sql = "INSERT INTO NetDisk2.tbl_file (file_sha1,file_name,file_size,file_addr,status) VALUES('" + FileUtil::sha1File(filepath.c_str()) + "','" + fileInfo.first + "'," + std::to_string(fileInfo.second.size()) + ",'" + filepath + "', 0);";
    sql += "INSERT INTO NetDisk2.tbl_user_file (user_name, file_sha1, file_name, file_size) VALUES ('" + username + "','" + FileUtil::sha1File(filepath.c_str()) + "','" + fileInfo.first + "'," + std::to_string(fileInfo.second.size()) + ");";
    fprintf(stderr, "sql = %s\n", sql.c_str());
    resp->MySQL("mysql://root:gaoyaguo971118@localhost", sql, [](Json* pjson) {
        fprintf(stderr, "out = %s\n", pjson->dump().c_str());
    });
    // fprintf(stderr,"sql = %s\n", sql.c_str());
    if (_config.storeType == OSS) {
        OSSInfo info;
        ClientConfiguration conf;
        OssClient client(info.Endpoint, info.AccessKeyId, info.AccessKeySecret, conf);
        std::string osspath = "disk/" + FileUtil::sha1File(filepath.c_str());
        if (_config.isAsyncTransferEnable == true) {
            auto outcome = client.PutObject(info.Bucket, osspath, filepath);  //把本地的filepath存入OSS的osspath
            if (!outcome.isSuccess()) {
                fprintf(stderr, "PutObject fail, code = %s, message = %s, request id = %s\n", outcome.error().Code().c_str(), outcome.error().Message().c_str(), outcome.error().RequestId().c_str());
            }
        }
    }
    resp->set_status_code("302");
    resp->headers["Location"] = "/file/upload/success";
}

void FileCallbackLib::fileUploadSuccess(const wfrest::HttpReq* req, wfrest::HttpResp* resp) {
    resp->String("Upload success");
}

void FileCallbackLib::fileDownloadGet(const wfrest::HttpReq* req, wfrest::HttpResp* resp) {
    // // /file/download?filehash=aaf4c61ddcc5e8a2dabede0f3b482cd9aea9434d&filename=1.txt&filesize=5
    auto fileInfo = req->query_list();
    // std::string filesha1 = fileInfo["filehash"];
    std::string filename = fileInfo["filename"];
    // int filesize = std::stoi(fileInfo["filesize"]);
    // std::string filepath = "../tmp/" + filename;

    // int fd = open(filepath.c_str(),O_RDONLY);
    // int size = lseek(fd,0,SEEK_END);
    // lseek(fd,0,SEEK_SET);
    // std::unique_ptr<char []> buf(new char[size]);
    // read(fd,buf.get(),size);

    // resp->append_output_body(buf.get(),size);
    // resp->headers["Content-Type"] = "application/octect-stream";
    // resp->headers["content-disposition"] = "attachment;filename="+filename;
    resp->set_status_code("302");
    resp->headers["Location"] = "http://10.211.55.5:1235/" + filename;
}

void FileCallbackLib::fileQueryPost(const wfrest::HttpReq* req, wfrest::HttpResp* resp, SeriesWork* series) {
    // 解析用户请求
    auto userInfo = req->query_list();
    std::string username = userInfo["username"];
    auto form_kv = req->form_kv();
    std::string limit = form_kv["limit"];
    // 根据用户名查tbl_user_file
    std::string sql = "SELECT file_sha1,file_name,file_size,upload_at,last_update FROM NetDisk2.tbl_user_file WHERE user_name = '" + username + "' LIMIT " + limit + ";";
    // fprintf(stderr,"sql = %s\n", sql.c_str());
    auto mysqlTask = WFTaskFactory::create_mysql_task("mysql://root:gaoyaguo971118@localhost", 0, [resp](WFMySQLTask* mysqlTask) {
        auto respMysql = mysqlTask->get_resp();
        protocol::MySQLResultCursor cursor(respMysql);
        std::vector<std::vector<protocol::MySQLCell>> rows;
        cursor.fetch_all(rows);
        Json respArr;
        for (auto& row : rows) {
            Json fileJson;
            // row[0] file_sha1
            fileJson["FileHash"] = row[0].as_string();
            // row[1] file_name
            fileJson["FileName"] = row[1].as_string();
            // row[2] file_size
            fileJson["FileSize"] = row[2].as_ulonglong();
            // row[3] upload_at
            fileJson["UploadAt"] = row[3].as_datetime();
            // row[4] lastupdate
            fileJson["LastUpdated"] = row[4].as_datetime();
            respArr.push_back(fileJson);
        }
        // fprintf(stderr,"out = %s\n", respArr.dump().c_str());
        resp->String(respArr.dump());
    });
    mysqlTask->get_req()->set_query(sql);
    series->push_back(mysqlTask);
}

void FileCallbackLib::fileDownloadUrlPost(const wfrest::HttpReq* req, wfrest::HttpResp* resp, SeriesWork* series) {
    auto fileInfo = req->query_list();
    std::string sha1 = fileInfo["filehash"];
    std::string sql = "SELECT file_name FROM NetDisk2.tbl_file WHERE file_sha1='" + sha1 + "';";
    auto mysqlTask = WFTaskFactory::create_mysql_task("mysql://root:gaoyaguo971118@localhost", 0, [resp](WFMySQLTask* mysqlTask) {
        auto respMysql = mysqlTask->get_resp();
        protocol::MySQLResultCursor cursor(respMysql);
        std::vector<std::vector<protocol::MySQLCell>> rows;
        cursor.fetch_all(rows);
        resp->String("http://10.211.55.5:1235/" + rows[0][0].as_string());
    });
    mysqlTask->get_req()->set_query(sql);
    series->push_back(mysqlTask);
}