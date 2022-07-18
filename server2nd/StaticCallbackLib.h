#ifndef __STATICCALLBACKLIB_H__
#define __STATICCALLBACKLIB_H__

#include <wfrest/HttpServer.h>

class StaticCallbackLib {
public:
    void staticSignin(const wfrest::HttpReq* req, wfrest::HttpResp* resp) {
        resp->File("../static/view/signin.html");
    }
    void staticHome(const wfrest::HttpReq* req, wfrest::HttpResp* resp) {
        resp->File("../static/view/home.html");
    }
    void staticAuthJs(const wfrest::HttpReq* req, wfrest::HttpResp* resp) {
        resp->File("../static/js/auth.js");
    }
    void staticAvatarImg(const wfrest::HttpReq* req, wfrest::HttpResp* resp) {
        resp->File("../static/img/avatar.jpeg");
    }
};

#endif