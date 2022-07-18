
#include "signup.srpc.h"
#include "workflow/WFFacilities.h"

using namespace srpc;

static WFFacilities::WaitGroup wait_group(1);

void sig_handler(int signo) {
    wait_group.done();
}

// context和series对接，获取一些连接信息
static void signup_done(RespSignup* response, srpc::RPCContext* context) {
	// 可以访问resp
	fprintf(stderr, "code = %d, message = %s\n", response->code(), response->message().c_str());
}

int main() {
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    // 服务端ip和端口
    const char* ip = "127.0.0.1";
    unsigned short port = 1412;

    // 服务端在客户端的代理
    UserService::SRPCClient client(ip, port);
    // 准备rpc的传入参数
    ReqSignup signup_req;
    signup_req.set_username("admin1");
    signup_req.set_password("1234");
    // 通过代理对象的方法，调用rpc
    // 当服务端回复响应的时候，调用signup_done函数
    // 使用rpc的形式和普通函数的调用是一样的
    // client.Signup(&signup_req, signup_done);

	// 使用任务的形式来使用客户端
	auto rpcTask = client.create_Signup_task(signup_done);
	// 修改任务的属性以设置rpc的传入参数
	rpcTask->serialize_input(&signup_req);
	// 启动任务
	rpcTask->start();
    wait_group.wait();
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
