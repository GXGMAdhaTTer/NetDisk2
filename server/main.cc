#include <fcntl.h>
#include <gxgfunc.h>
#include <wfrest/HttpMsg.h>
#include <wfrest/HttpServer.h>
#include <workflow/MySQLResult.h>
#include <workflow/MySQLUtil.h>
#include <workflow/WFFacilities.h>
#include <workflow/WFTaskFactory.h>
#include <functional>
#include <wfrest/json.hpp>
#include "FileUtil.h"

using namespace std::placeholders;
using Json = nlohmann::json;

using UserCallbackType = std::function<void(const wfrest::HttpReq*, wfrest::HttpResp*)>;

static WFFacilities::WaitGroup waitGroup(1);

void sigHandler(int num) {
    waitGroup.done();
    fprintf(stderr, "wait group is done\n");
}

int main() {
    signal(SIGINT, sigHandler);
    wfrest::HttpServer server;
    server.GET("/file/upload", [](const wfrest::HttpReq* req, wfrest::HttpResp* resp) {
        resp->File("../static/view/index.html");
    });
    server.POST("/file/upload", [](const wfrest::HttpReq* req, wfrest::HttpResp* resp) {
        //读取文件内容 解析form-data类型的请求报文
        using Form = std::map<std::string, std::pair<std::string, std::string>>;
        Form& form = req->form();  //不能用const
        auto fileInfo = form["file"];
        std::string filepath = "../tmp/" + fileInfo.first;
        int fd = open(filepath.c_str(), O_RDWR | O_CREAT | O_EXCL, 0666);
        if (fd < 0) {
            resp->set_status_code("500");
            return;
        }
        // try catch
        /* 方案1 同步*/
        int ret = write(fd, fileInfo.second.c_str(), fileInfo.second.size());
        close(fd);
        fprintf(stderr, "sha1 = %s\n", FileUtil::sha1File(filepath.c_str()).c_str());
        /* 方案2 pwrite
        auto pwriteTask = WFTaskFactory::create_pwrite_task(fd, fileInfo.second.c_str(), fileInfo.second.size(), 0, callback);
        series->push_back(pwriteTask); //需要三参数版本的POST
        //把上传之后的逻辑写到callback当中
        */
        std::string sql = "INSERT INTO NetDisk2.tbl_file (file_sha1, file_name, file_size, file_addr, status) VALUES('" + FileUtil::sha1File(filepath.c_str()) + "','" + fileInfo.first + "'," + std::to_string(fileInfo.second.size()) + ", '" + filepath + "', 0)";
        resp->MySQL("mysql://root:gaoyaguo971118@localhost", sql, [](Json* pjson) {
            fprintf(stderr, "out = %s\n", pjson->dump().c_str());
        });
        fprintf(stderr, "sql = %s\n", sql.c_str());
        resp->set_status_code("302");
        resp->headers["Location"] = "/file/upload/success";
    });
    server.GET("/file/upload/success", [](const wfrest::HttpReq* req, wfrest::HttpResp* resp) {
        resp->String("Upload success");
    });
    server.GET("/file/download", [](const wfrest::HttpReq* req, wfrest::HttpResp* resp) {
        // /file/download?filehash=10c8e9c9aaf9788b7826884c2851fa6253cd4bc7&filename=1.txt&filesize=10
        auto fileInfo = req->query_list();
        std::string fileha1 = fileInfo["filehash"];
        std::string filename = fileInfo["filename"];
        int filesize = std::stoi(fileInfo["filesize"]);

        /* std::string filepath = "tmp/" + filename; */

        /* int fd = open(filepath.c_str(), O_RDONLY); */
        /* std::unique_ptr<char []> buf(new char[filesize]); */
        /* read(fd, buf.get(), filesize); */
        /* close(fd); */
        /* resp->append_output_body(buf.get(), filesize); */
        /* resp->headers["Content-Type"] = "application/octect-stream"; */
        /* resp->headers["Content-Disposition"] = "attachment;filename="+filename; */
        resp->set_status_code("302");
        resp->headers["Location"] = "http://10.211.55.5:1235/" + filename;
    });
    /* User user; */
    /* UserCallbackType signupFunc = std::bind(&User::userSignup, &user, _1, _2); */
    /* UserCallbackType signinFunc = std::bind(&User::userSignin, &user, _1, _2); */
    /* UserCallbackType infoFunc = std::bind(&User::userInfo, &user, _1, _2); */
    /* server.POST("/user/signup", signupFunc); */
    /* server.POST("/user/signin", signinFunc); */
    /* server.GET("/user/info", infoFunc); */

    if (server.track().start(1234) == 0) {  // track打印请求
        server.list_routes();               //列出当前服务端支持哪些方法和路径
        waitGroup.wait();
        server.stop();
        return 0;
    } else {
        fprintf(stderr, "can nod start server\n");
        return EXIT_FAILURE;
    }
}
