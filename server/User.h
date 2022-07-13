#include <gxgfunc.h>
#include <functional>
#include <wfrest/HttpMsg.h>
#include <wfrest/HttpServer.h>
#include <workflow/MySQLResult.h>
#include <workflow/MySQLUtil.h>
#include <workflow/WFFacilities.h>
#include <wfrest/json.hpp>

class User {
public:
    User() {}
    void userSignup(const wfrest::HttpReq*, wfrest::HttpResp*);
    void userInfo(const wfrest::HttpReq*, wfrest::HttpResp*);
    void userSignin(const wfrest::HttpReq*, wfrest::HttpResp*);
};
