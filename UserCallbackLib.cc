#include "UserCallbackLib.h"
#include <workflow/MySQLMessage.h>
#include <workflow/MySQLResult.h>
#include <wfrest/json.hpp>
#include "Token.h"
#include "UserInfo.h"

using Json = nlohmann::json;

void UserCallbackLib::userSignupGet(const wfrest::HttpReq* req, wfrest::HttpResp* resp) {
    resp->File("../static/view/signup.html");
}

void UserCallbackLib::userSignupPost(const wfrest::HttpReq* req, wfrest::HttpResp* resp, SeriesWork* series) {
    // 1 按urlencoded的形式去解析post报文体
    std::map<std::string, std::string>& form_kv = req->form_kv();
    std::string username = form_kv["username"];
    std::string password = form_kv["password"];
    // 2 把密码进行加密
    std::string salt = "12345678";
    char* encryptPassword = crypt(password.c_str(), salt.c_str());
    // fprintf(stderr,"encryptPassword = %s\n", encryptPassword);
    // 3 把用户信息插入到数据库
    std::string sql = "INSERT INTO NetDisk2.tbl_user (user_name,user_pwd) VALUES( '" + username + "','" + encryptPassword + "');";
    fprintf(stderr, "sql = %s\n", sql.c_str());
    // create_mysql_task
    auto mysqlTask = WFTaskFactory::create_mysql_task("mysql://root:gaoyaguo971118@localhost", 0, [](WFMySQLTask* mysqlTask) {
        // 4 回复一个SUCCESS给前端
        wfrest::HttpResp* resp2client = static_cast<wfrest::HttpResp*>(mysqlTask->user_data);
        if (mysqlTask->get_state() != WFT_STATE_SUCCESS) {
            fprintf(stderr, "error msg:%s\n", WFGlobal::get_error_string(mysqlTask->get_state(), mysqlTask->get_error()));
            resp2client->append_output_body("FAIL", 4);
            return;
        }

        protocol::MySQLResponse* resp = mysqlTask->get_resp();
        protocol::MySQLResultCursor cursor(resp);

        // 检查语法错误
        if (resp->get_packet_type() == MYSQL_PACKET_ERROR) {
            fprintf(stderr, "error_code = %d msg = %s\n", resp->get_error_code(), resp->get_error_msg().c_str());
            resp2client->append_output_body("FAIL", 4);
            return;
        }

        if (cursor.get_cursor_status() == MYSQL_STATUS_OK) {
            //写指令，执行成功
            fprintf(stderr, "OK. %llu rows affected. %d warnings. insert_id = %llu.\n",
                    cursor.get_affected_rows(), cursor.get_warnings(), cursor.get_insert_id());
            if (cursor.get_affected_rows() == 1) {
                resp2client->append_output_body("SUCCESS", 7);
                return;
            }
        }
    });
    mysqlTask->get_req()->set_query(sql);
    mysqlTask->user_data = resp;
    // push_back
    series->push_back(mysqlTask);
}

void UserCallbackLib::userSigninGet(const wfrest::HttpReq* req, wfrest::HttpResp* resp) {
    resp->File("../static/view/signin.html");
}

void UserCallbackLib::userSigninPost(const wfrest::HttpReq* req, wfrest::HttpResp* resp, SeriesWork* series) {
    // 1 解析用户请求
    std::map<std::string, std::string>& form_kv = req->form_kv();
    std::string username = form_kv["username"];
    std::string password = form_kv["password"];
    // 2 查询数据库
    std::string url = "mysql://root:gaoyaguo971118@localhost";
    std::string sql = "SELECT user_pwd FROM NetDisk2.tbl_user WHERE user_name = '" + username + "' LIMIT 1;";
    auto readTask = WFTaskFactory::create_mysql_task(url, 0, [](WFMySQLTask* readTask) {
        //提取readTask的结果
        auto resp = readTask->get_resp();
        protocol::MySQLResultCursor cursor(resp);

        std::vector<std::vector<protocol::MySQLCell>> rows;
        cursor.fetch_all(rows);

        std::string nowPassword = rows[0][0].as_string();
        fprintf(stderr, "nowPassword = %s\n", nowPassword.c_str());

        UserInfo* userinfo = static_cast<UserInfo*>(series_of(readTask)->get_context());
        char* inPassword = crypt(userinfo->password.c_str(), "12345678");
        fprintf(stderr, "inPassword = %s\n", inPassword);
        if (strcmp(nowPassword.c_str(), inPassword) != 0) {
            userinfo->resp->append_output_body("FAIL", 4);
            return;
        }
        // 3 生成一个token，存入数据库当中
        //  用户的信息->加密得到密文 拼接上 登录时间
        Token usertoken(userinfo->username, "12345678");
        // fprintf(stderr,"token = %s\n",usertoken.token.c_str());
        userinfo->token = usertoken.token;
        // 存入数据库当中
        std::string url = "mysql://root:gaoyaguo971118@localhost";
        std::string sql = "REPLACE INTO NetDisk2.tbl_user_token (user_name,user_token) VALUES ('" + userinfo->username + "', '" + usertoken.token + "');";
        auto writeTask = WFTaskFactory::create_mysql_task(url, 0, [](WFMySQLTask* writeTask) {
            UserInfo* userinfo = static_cast<UserInfo*>(series_of(writeTask)->get_context());
            Json uinfo;
            uinfo["Username"] = userinfo->username;
            uinfo["Token"] = userinfo->token;
            uinfo["Location"] = "/static/view/home.html";
            Json respInfo;
            respInfo["code"] = 0;
            respInfo["msg"] = "OK";
            respInfo["data"] = uinfo;
            userinfo->resp->String(respInfo.dump());
        });
        writeTask->get_req()->set_query(sql);
        series_of(readTask)->push_back(writeTask);
    });
    readTask->get_req()->set_query(sql);
    series->push_back(readTask);
    UserInfo* userinfo = new UserInfo;
    userinfo->username = username;
    userinfo->password = password;
    userinfo->resp = resp;
    series->set_context(userinfo);
    // 在序列的回调函数当中释放
    series->set_callback([](const SeriesWork* series) {
        UserInfo* userinfo = static_cast<UserInfo*>(series->get_context());
        delete userinfo;
        fprintf(stderr, "userinfo is deleted\n");
    });
}

void UserCallbackLib::userInfoPost(const wfrest::HttpReq* req, wfrest::HttpResp* resp, SeriesWork* series) {
    // 1 解析用户请求
    auto userInfo = req->query_list();
    // 2 校验token是否合法 --> 拦截器
    // 3 根据用户信息，查询sql
    std::string sql = "SELECT user_name, signup_at FROM NetDisk2.tbl_user WHERE user_name='" + userInfo["username"] + "' LIMIT 1;";
    auto mysqlTask = WFTaskFactory::create_mysql_task("mysql://root:gaoyaguo971118@localhost", 0, [resp](WFMySQLTask* mysqlTask) {
        auto respMysql = mysqlTask->get_resp();
        protocol::MySQLResultCursor cursor(respMysql);
        std::vector<std::vector<protocol::MySQLCell>> rows;
        cursor.fetch_all(rows);
        // fprintf(stderr,"username = %s, signupat = %s\n", rows[0][0].as_string().c_str(), rows[0][1].as_datetime().c_str());
        Json uInfo;
        uInfo["Username"] = rows[0][0].as_string();
        uInfo["SignupAt"] = rows[0][1].as_datetime();
        Json respInfo;
        respInfo["data"] = uInfo;
        respInfo["code"] = 0;
        respInfo["msg"] = "OK";
        resp->String(respInfo.dump());
    });
    mysqlTask->get_req()->set_query(sql);
    series->push_back(mysqlTask);
}