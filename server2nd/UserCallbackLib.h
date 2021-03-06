#ifndef __USERCALLBACKLIB_H__
#define __USERCALLBACKLIB_H__

#include <wfrest/HttpServer.h>

class UserCallbackLib {
public:
    void userSignupGet(const wfrest::HttpReq *req, wfrest::HttpResp *resp);
    void userSignupPost(const wfrest::HttpReq *req, wfrest::HttpResp *resp, SeriesWork *series);
    void userSigninGet(const wfrest::HttpReq *req, wfrest::HttpResp *resp);
    void userSigninPost(const wfrest::HttpReq *req, wfrest::HttpResp *resp, SeriesWork *series);
    void userInfoPost(const wfrest::HttpReq *req, wfrest::HttpResp *resp, SeriesWork *series);
};

#endif