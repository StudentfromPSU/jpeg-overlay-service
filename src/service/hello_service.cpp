#include "service/hello_service.hpp"

HelloService::HelloService()
{
}

grpc::Status HelloService::SayHello(
    grpc::ServerContext* context,
    const hw::HelloRequest* request,
    hw::HelloReply* response)
{
    response->set_message("Hello, " + request->name());

    return grpc::Status::OK;
}