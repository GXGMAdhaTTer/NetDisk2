#include <wfrest/HttpServer.h>
#include <workflow/MySQLMessage.h>
#include <workflow/MySQLResult.h>
#include <workflow/WFFacilities.h>
#include <functional>
#include <wfrest/json.hpp>
#include "FileCallbackLib.h"
#include "FileUtil.h"
#include "OSS.h"
#include "StaticCallbackLib.h"
#include "Token.h"
#include "UserCallbackLib.h"
#include "UserInfo.h"
#include "linuxheader.h"

using namespace AlibabaCloud::OSS;
using Json = nlohmann::json;
using CallbackType = std::function<void(const wfrest::HttpReq* req, wfrest::HttpResp* resp)>;
using CallbackTypeTri = std::function<void(const wfrest::HttpReq* req, wfrest::HttpResp* resp, SeriesWork* series)>;
using namespace std::placeholders;

static WFFacilities::WaitGroup waitGroup(1);

void sigHandler(int num) {
    waitGroup.done();
    fprintf(stderr, "wait group is done\n");
}

int main() {
    signal(SIGINT, sigHandler);
    InitializeSdk();
    Config config;

    wfrest::HttpServer server;
    FileCallbackLib fileCallbackLib(config);
    CallbackType fileUploadGetCallback = std::bind(&FileCallbackLib::fileUploadGet, &fileCallbackLib, _1, _2);
    CallbackType fileUploadPostCallback = std::bind(&FileCallbackLib::fileUploadPost, &fileCallbackLib, _1, _2);
    CallbackType fileUploadSuccessCallback = std::bind(&FileCallbackLib::fileUploadSuccess, &fileCallbackLib, _1, _2);
    CallbackType fileDownloadGetCallback = std::bind(&FileCallbackLib::fileDownloadGet, &fileCallbackLib, _1, _2);
    CallbackTypeTri fileQueryPostCallback = std::bind(&FileCallbackLib::fileQueryPost, &fileCallbackLib, _1, _2, _3);
    CallbackTypeTri fileDownloadUrlPostCallback = std::bind(&FileCallbackLib::fileDownloadUrlPost, &fileCallbackLib, _1, _2, _3);
    server.GET("/file/upload", fileUploadGetCallback);
    server.POST("/file/upload", fileUploadPostCallback);
    server.GET("/file/upload/success", fileUploadSuccessCallback);
    server.GET("/file/download", fileDownloadGetCallback);
    server.POST("/file/query", fileQueryPostCallback);
    server.POST("/file/downloadurl", fileDownloadUrlPostCallback);

    UserCallbackLib userCallbackLib;
    CallbackType userSignupGetCallback = std::bind(&UserCallbackLib::userSignupGet, &userCallbackLib, _1, _2);
    CallbackTypeTri userSignupPostCallback = std::bind(&UserCallbackLib::userSignupPost, &userCallbackLib, _1, _2, _3);
    CallbackType userSigninGetCallback = std::bind(&UserCallbackLib::userSigninGet, &userCallbackLib, _1, _2);
    CallbackTypeTri userSigninPostCallback = std::bind(&UserCallbackLib::userSigninPost, &userCallbackLib, _1, _2, _3);
    CallbackTypeTri userInfoPostCallback = std::bind(&UserCallbackLib::userInfoPost, &userCallbackLib, _1, _2, _3);
    server.GET("/user/signup", userSignupGetCallback);
    server.POST("/user/signup", userSignupPostCallback);
    server.GET("/user/signin", userSigninGetCallback);
    server.POST("/user/signin", userSigninPostCallback);
    server.POST("/user/info", userInfoPostCallback);

    StaticCallbackLib staticCallbackLib;
    CallbackType staticSigninCallback = std::bind(&StaticCallbackLib::staticSignin, &staticCallbackLib, _1, _2);
    CallbackType staticHomeCallback = std::bind(&StaticCallbackLib::staticHome, &staticCallbackLib, _1, _2);
    CallbackType staticAuthJsCallback = std::bind(&StaticCallbackLib::staticAuthJs, &staticCallbackLib, _1, _2);
    CallbackType staticAvatarImgCallback = std::bind(&StaticCallbackLib::staticAvatarImg, &staticCallbackLib, _1, _2);
    server.GET("/static/view/signin.html", staticSigninCallback);
    server.GET("/static/view/home.html", staticHomeCallback);
    server.GET("/static/js/auth.js", staticAuthJsCallback);
    server.GET("/static/img/avatar.jpeg", staticAvatarImgCallback);

    if (server.track().start(1234) == 0) {
        server.list_routes();
        waitGroup.wait();
        server.stop();
        ShutdownSdk();
    } else {
        fprintf(stderr, "can not start server!\n");
        return -1;
    }
}
