#include <string> 
#include <wfrest/HttpServer.h>
// serverTask向mysqlTask传递信息
struct UserInfo{
    std::string username;
    std::string password;
    std::string token;
    wfrest::HttpResp *resp;
};
