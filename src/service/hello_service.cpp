#include "service/hello_service.hpp"
#include <iostream>
//#include <thread>
//#include <chrono>
HelloService::HelloService(std::shared_ptr<ConnectionLimiter> limiter) : limiter_(limiter)
{
}

grpc::Status HelloService::SayHello(
    grpc::ServerContext*,
    const hw::HelloRequest* request,
    hw::HelloReply* response)
{
    if (limiter_ && !limiter_->TryAcquire())
    {
        std::cout << "Connection rejected: server busy (active: " << limiter_->GetActiveConnections()
                  << "/" << limiter_->GetMaxConnections() << ")" << std::endl;
        return grpc::Status(grpc::StatusCode::RESOURCE_EXHAUSTED, "Server is busy");
    }

    ConnectionGuard guard(limiter_, limiter_ != nullptr);

    std::cout << "Processing SayHello for: " << request->name() << " (active: "
              << (limiter_ ? limiter_->GetActiveConnections() : 0) << ")" << std::endl;

    //testing...
    //std::this_thread::sleep_for(std::chrono::seconds(10));
    response->set_message("Hello, " + request->name());

    return grpc::Status::OK;
}