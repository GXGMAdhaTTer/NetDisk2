#include "linuxheader.h"
#include "ppconsul/agent.h"
#include "ppconsul/consul.h"
#include "signup.srpc.h"
#include "workflow/WFFacilities.h"

using namespace srpc;

static WFFacilities::WaitGroup wait_group(1);

void sig_handler(int signo) {
    wait_group.done();
}

// 必须继承，并override Signup函数
class UserServiceServiceImpl : public UserService::Service {
public:
    void Signup(ReqSignup* request, RespSignup* response, srpc::RPCContext* ctx) override {
        std::string username = request->username();
        std::string password = request->password();
        std::string salt = "12345678";
        // 2 把密码进行加密
        char* encryptPassword = crypt(password.c_str(), salt.c_str());
        // fprintf(stderr,"encryptPassword = %s\n", encryptPassword);
        // 3 把用户信息插入到数据库
        std::string sql = "INSERT INTO NetDisk2.tbl_user (user_name,user_pwd) VALUES( '" + username + "','" + encryptPassword + "');";
        auto mysqlTask = WFTaskFactory::create_mysql_task("mysql://root:gaoyaguo971118@localhost", 0, [response](WFMySQLTask* mysqlTask) {
            if (mysqlTask->get_state() == WFT_STATE_SUCCESS) {
                response->set_message(("SIGNUP SUCCESS"));
                response->set_code(0);
            } else {
                response->set_message(("SIGNUP FAIL"));
                response->set_code(1);
            }
        });
        // 通过ctx找到rpcTask所在的seris
        mysqlTask->get_req()->set_query(sql);
        ctx->get_series()->push_back(mysqlTask);
    };
};

void timerCallback(WFTimerTask* timerTask) {
	fprintf(stderr, "time = %ld\n", time(NULL));
	ppconsul::agent::Agent* pagent = static_cast<ppconsul::agent::Agent*>(timerTask->user_data);
	pagent->servicePass("SignupService1");
	auto nextTask = WFTaskFactory::create_timer_task(5000000, timerCallback);
	nextTask->user_data = pagent;
	series_of(timerTask)->push_back(nextTask);
}
int main() {
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    unsigned short port = 1412;
    SRPCServer server;

    // 注册一个服务
    UserServiceServiceImpl userservice_impl;
    server.add_service(&userservice_impl);

    // 启动一个server
    server.start(port);

    // 将本服务注册到consul之中
    // 找到dc1注册中心
    ppconsul::Consul consul("127.0.0.1:8500", ppconsul::kw::dc = "dc1");
    // 创建一个用来访问注册中心的agent
    ppconsul::agent::Agent agent(consul);
    agent.registerService(ppconsul::agent::kw::name = "SignupService1",
                          ppconsul::agent::kw::address = "127.0.0.1",
                          ppconsul::agent::kw::id = "SignupService1",
                          ppconsul::agent::kw::port = 1412,
                          ppconsul::agent::kw::check = ppconsul::agent::TtlCheck{std::chrono::seconds(10)});
	// agent.deregisterService("SignupService1");
	// agent.servicePass("SignupService1");
	auto timerTask = WFTaskFactory::create_timer_task(5000000, timerCallback);
	timerTask->user_data = &agent;
	timerTask->start();
    wait_group.wait();
    server.stop();
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
